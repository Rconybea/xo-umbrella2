// xo-websock/example/introspect/mount-origin/route_edges.js
//
// member edges routed around boxes already placed: libavoid (orthogonal
// routing), behind one function.  Placement never sees a member edge, so
// opening a box cannot rearrange distant ones.
// See .xo-backlog/introspect-layout/spec.md

const avoid_module_url = "https://cdn.jsdelivr.net/npm/libavoid-js@0.4.5/dist/index.js";
const avoid_wasm_url = "https://cdn.jsdelivr.net/npm/libavoid-js@0.4.5/dist/libavoid.wasm";

const avoid_shape_buffer = 8;   // an edge keeps this far from any box it passes.  12 is
                                // too much: two boxes in one group sit closer than twice
                                // that, leaving no path between them
const avoid_nudge = 10;         // between parallel stretches of different edges

// libavoid's ConnDirFlags: the way an edge leaves / enters a pin
const avoid_dir_up = 1;
const avoid_dir_down = 2;

/** libavoid, loaded: the module, then its wasm.  Rejects with an Error
 *  naming libavoid and the url that failed
 **/
async function load_router() {
    let url = avoid_module_url;
    try {
        const {AvoidLib} = await import(url);
        url = avoid_wasm_url;
        await AvoidLib.load(url);
        return AvoidLib.getInstance();
    } catch (e) {
        throw new Error(`libavoid: ${url}: ${e && e.message || e}`);
    }
}

/** orthogonal routes for @p edges around @p boxes, which stay put.
 *
 *  @p avoid   libavoid, as load_router() resolves it
 *  @p boxes   Map id -> {x, y, w, h}, one coordinate frame
 *  @p edges   [{key, from, out_x, to}]: from box @p from's bottom edge,
 *             @p out_x from its left, to box @p to's top edge
 *  @p in_x    where an edge enters its target, from the target's left: one
 *             entry point per box, shared by every edge into it
 *
 *  @return Map key -> [{x, y}], in the frame of @p boxes
 *
 *  Synchronous: a redraw cannot be superseded half-way through.  The router
 *  and every wasm object made here are freed before it returns
 **/
function route_edges(avoid, boxes, edges, in_x) {
    const routes = new Map();
    if (edges.length === 0)
        return routes;

    // wasm objects the router copies rather than owns: freed at the end
    const temps = [];
    const temp = (obj) => { temps.push(obj); return obj; };
    const point = (x, y) => temp(new avoid.Point(x, y));

    const router = new avoid.Router(avoid.OrthogonalRouting);
    try {
        router.setRoutingParameter(avoid.shapeBufferDistance, avoid_shape_buffer);
        router.setRoutingParameter(avoid.idealNudgingDistance, avoid_nudge);

        // the router owns shapes, their pins and connectors: freed with it
        const shapes = new Map();
        for (const [id, b] of boxes)
            shapes.set(id, new avoid.ShapeRef(router,
                temp(new avoid.Rectangle(point(b.x, b.y), point(b.x + b.w, b.y + b.h)))));

        // a direction belongs to a pin, never to a bare point: ConnEnd(point,
        // dir) aborts the wasm for good.  One shared, non-exclusive entry pin
        // per box: a pin per edge, all at one spot, sends libavoid to the box
        // centre in a diagonal
        let next_class = 1;
        const in_pin = new Map();   // box id -> its entry pin's class
        const conns = [];
        for (const e of edges) {
            const src = shapes.get(e.from);
            const dst = shapes.get(e.to);
            const out_class = next_class++;
            new avoid.ShapeConnectionPin(src, out_class, e.out_x, boxes.get(e.from).h,
                                         false, 0, avoid_dir_down);
            if (!in_pin.has(e.to)) {
                const in_class = next_class++;
                const pin = new avoid.ShapeConnectionPin(dst, in_class, in_x, 0,
                                                         false, 0, avoid_dir_up);
                pin.setExclusive(false);
                in_pin.set(e.to, in_class);
            }
            conns.push({key: e.key,
                        conn: new avoid.ConnRef(router,
                                                temp(new avoid.ConnEnd(src, out_class)),
                                                temp(new avoid.ConnEnd(dst, in_pin.get(e.to))))});
        }

        router.processTransaction();

        for (const {key, conn} of conns) {
            const line = conn.displayRoute();
            const pts = [];
            for (let i = 0; i < line.size(); ++i) {
                const p = line.get_ps(i);
                pts.push({x: p.x, y: p.y});
            }
            routes.set(key, pts);
        }
        return routes;
    } finally {
        // after a wasm abort, destroy() throws too: that must not replace
        // the error that brought us here.  Nothing is left to free then
        try {
            avoid.destroy(router);
            for (const obj of temps)
                avoid.destroy(obj);
        } catch (e) {
            console.error("route_edges: freeing the router failed:", e);
        }
    }
}
