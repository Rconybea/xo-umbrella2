// open the server box by a real left-click; then a constructed snapshot with
// a ref member and a nested member, to exercise ports and nesting
import fs from "node:fs";
const [,, cdp_port, port, out1, out2] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const clients = [];
for (const streams of [["/demo/1"]]) {
    const w = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
    await new Promise(r => w.onopen = r);
    for (const st of streams) w.send(JSON.stringify({cmd: "subscribe", stream: st}));
    clients.push(w);
}
await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl);
await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => (await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true})).result.result?.value;
const VIS = `[...document.querySelectorAll("path.edge.member")].filter(p => [...document.querySelectorAll("text.row")].some(t => t.__data__ && p.__data__.row_keys.includes(t.__data__.key)))`;   // edges whose row is open
// the server's first_endpoint_ edge: its other open rows (ws_config_,
// url_router_, session_table_) have edges too -- to their nested boxes
const FE = `${VIS}.filter(p => p.__data__.row_keys.includes("server/first_endpoint_"))`;
const mouse = async (type, x, y, button) => call("Input.dispatchMouseEvent", {type, x, y, button, clickCount: 1});
const click = async (x, y) => { await mouse("mouseMoved", x, y, "none"); await mouse("mousePressed", x, y, "left"); await mouse("mouseReleased", x, y, "left"); };
const overlaps = `(() => { const r = [...document.querySelectorAll("g.node > rect")].map(e => e.getBoundingClientRect()); let n = 0;
  for (let i = 0; i < r.length; i++) for (let j = i + 1; j < r.length; j++) { const a = r[i], b = r[j];
    if (a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom) n++; } return n; })()`;
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok = " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1400, height: 1100, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`!!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(300);
// only the Webserver box is shown by default: show every box's children
await ev(`document.getElementById("show-all").click()`);
for (let i = 0; i < 100 && !(await ev(`document.querySelectorAll("path.edge").length > 0`)); i++) await sleep(100);
await sleep(500);
// Show all centres the drawing at 100%: wider than the viewport, its sides
// are off screen.  Fit, so the boxes clicked below are in view
await ev(`document.getElementById("fit").click()`); await sleep(800);

const box = async (sel) => ev(`(() => { const r = document.querySelector(${JSON.stringify(sel)} + " > rect").getBoundingClientRect(); return {x: r.left + 20, y: r.top + 15, h: r.height}; })()`);
let b = await box("g.node.server");
check(await ev(`document.querySelector("g.node.server").classList.contains("expandable")`), "server box is expandable");
check(await ev(`document.querySelectorAll("g.node.app").length`) === 0, "no ticker box");
await click(b.x, b.y);
await sleep(800);
const rows = await ev(`[...document.querySelectorAll("g.node.server text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`);
console.log(" =  rows:", JSON.stringify(rows));
check(rows.length === 6 && rows[0] === "ws_config_: ▾ (→)", "left-click opens: 6 member rows");
check(rows[1] === `listen_port_: ${port}`, "scalar row");
const b2 = await box("g.node.server");
const zk = await ev(`d3.zoomTransform(document.getElementById("graph")).k`);   // on screen: drawing units * k
check((b2.h - b.h) / zk > 6 * 18 - 1, `box grew: ${b.h} -> ${b2.h} (zoom ${zk.toFixed(2)})`);
check((await ev(overlaps)) === 0, "no overlaps after reflow");
fs.writeFileSync(out1, Buffer.from((await call("Page.captureScreenshot", {format: "png", clip: {x: 0, y: 0, width: 1400, height: 1100, scale: 1}})).result.data, "base64"));

// constructed: a ref member (to an endpoint), and a nested member with members
await ev(`(() => { const e = JSON.parse(JSON.stringify(last_event));
  const ep = e.server.endpoints[0];
  e.server._members_.push({_name_: "first_endpoint_", _canonical_type_: "xo::ref::intrusive_ptr<xo::web::DynamicEndpoint>", _short_type_: "rp<DynamicEndpoint>", _metatype_: "pointer", _value_: {_ref_: ep._id_}});
  e.server._members_.push({_name_: "nested_", _canonical_type_: "xo::web::Outer", _short_type_: "Outer", _metatype_: "struct",
     _value_: {_name_: "Outer", _canonical_type_: "xo::web::Outer", _short_type_: "Outer", _members_: [{_name_: "inner_", _canonical_type_: "int", _short_type_: "int", _metatype_: "atomic", _value_: 42}]}});
  last_event = e; draw(e); return 1; })()`);
await sleep(800);
check(await ev(`${FE}.length`) === 1, "ref member: one member edge: " + await ev(`${FE}.length`));
const geo = await ev(`(() => { const p = ${FE}[0].getAttribute("d");
   const nums = p.match(/-?[0-9.]+/g).map(Number); const sx = nums[0], sy = nums[1], ex = nums[nums.length-2], ey = nums[nums.length-1];
   const srv = document.querySelector("g.node.server"); const st = srv.transform.baseVal[0].matrix; const sw = +srv.querySelector("rect").getAttribute("width");
   const row = [...srv.querySelectorAll("text.row")].find(t => t.querySelector("tspan").firstChild.nodeValue.startsWith("first_endpoint_"));
   const rowy = st.f + (+row.getAttribute("y")) - 4;
   const ep = document.querySelector("g.node.http"); const et = ep.transform.baseVal[0].matrix; const ew = +ep.querySelector("rect").getAttribute("width"); const eh = +ep.querySelector("rect").getAttribute("height");
   const sh = +srv.querySelector("rect").getAttribute("height");
   return {start_at_bottom_edge: Math.abs(sy - (st.f + sh)) < 2, start_near_left: sx >= st.e && sx <= st.e + 12 + 10 * 8,
           end_on_endpoint: ex >= et.e - 2 && ex <= et.e + ew + 2 && ey >= et.f - 2 && ey <= et.f + eh + 2,
           end_top_left: Math.abs(ey - et.f) < 3 && Math.abs(ex - (et.e + 24)) < 3}; })()`);
check(geo.start_at_bottom_edge && geo.start_near_left, `member edge leaves the box's bottom edge, near its left: ${JSON.stringify(geo)}`);
// hovering the row lights its edge; leaving restores it
const row_c = await ev(`(() => { const t = [...document.querySelectorAll("g.node.server text.row")].find(t => t.querySelector("tspan").firstChild.nodeValue.startsWith("first_endpoint_")).getBoundingClientRect(); return {x: t.left + 20, y: t.top + t.height / 2}; })()`);
await call("Input.dispatchMouseEvent", {type: "mouseMoved", x: row_c.x, y: row_c.y, button: "none"}); await sleep(150);
const hot = await ev(`({edge: ${FE}[0].classList.contains("hot"),
  rows: [...document.querySelectorAll("text.row.hot")].map(t => t.querySelector("tspan").firstChild.nodeValue),
  width: getComputedStyle(${FE}[0]).strokeWidth})`);
check(hot.edge && hot.width === "3px" && JSON.stringify(hot.rows) === '["first_endpoint_"]', "row hover: its edge and only its row lit: " + JSON.stringify(hot));
await call("Input.dispatchMouseEvent", {type: "mouseMoved", x: 5, y: 5, button: "none"}); await sleep(150);
check(!(await ev(`${FE}[0].classList.contains("hot") || !!document.querySelector("text.row.hot")`)), "mouse away: restored");
// hovering the edge lights its row
await ev(`${FE}[0].dispatchEvent(new MouseEvent("mouseover", {bubbles: true, relatedTarget: document.body}))`);
await ev(`${FE}[0].dispatchEvent(new MouseEvent("mouseenter"))`); await sleep(100);
check(await ev(`[...document.querySelectorAll("text.row.hot")].length === 1`), "edge hover: its row lit");
await ev(`${FE}[0].dispatchEvent(new MouseEvent("mouseleave"))`); await sleep(100);
check(geo.end_on_endpoint, "member edge ends at the referenced box");
check(geo.end_top_left, `... entering its top edge, 24px from its left: ${JSON.stringify(geo)}`);
check(await ev(`document.querySelectorAll("path.edge.link, path.edge.owns").length`) === 0
      && await ev(`document.querySelectorAll("g.node").length`) > 1, "ownership edges not drawn");
// nested_ (a struct with members) gets its own box; its row a ref to it
const srv_rows = `[...document.querySelectorAll("g.node.server text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`;
let r2 = await ev(srv_rows);
check(r2.some(t => t.startsWith("nested_: ▸ (→)")) && !r2.some(t => t.includes("inner_")), "nested member: a ref row, its box not drawn yet: " + r2.find(t => t.startsWith("nested_")));
await ev(`[...document.querySelectorAll("g.node.server text.row")].find(t => t.querySelector("tspan").firstChild.nodeValue.startsWith("nested_")).querySelector("tspan.rtoggle").dispatchEvent(new MouseEvent("click", {bubbles: true}))`);
await sleep(800);
r2 = await ev(srv_rows);
check(r2.some(t => t.startsWith("nested_: ▾ (→)")) && !r2.some(t => t.includes("inner_")), "its ▸ draws its box; nothing opens in place");
const NB = `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === "server/nested_")`;
check(await ev(`!!${NB} && ${NB}.classList.contains("nested") && ${NB}.__data__.group === "server"`), "box server/nested_: kind nested, grouped with the server");
await ev(`${NB}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(800);
check((await ev(`[...${NB}.querySelectorAll("text.row")].map(t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join(""))`)).some(t => t.startsWith("inner_: 42")), "opening that box: inner_ = 42");
check(await ev(`document.querySelector("g.node.server").classList.contains("open")`), "server still open (row click did not toggle the box)");
check((await ev(overlaps)) === 0, "no overlaps");
fs.writeFileSync(out2, Buffer.from((await call("Page.captureScreenshot", {format: "png", clip: {x: 0, y: 0, width: 1400, height: 1100, scale: 1}})).result.data, "base64"));

// Enter on the focused box closes it
await ev(`document.querySelector("g.node.server").focus()`);
await call("Input.dispatchKeyEvent", {type: "keyDown", key: "Enter", code: "Enter", windowsVirtualKeyCode: 13, text: "\r"});
await call("Input.dispatchKeyEvent", {type: "keyUp", key: "Enter", code: "Enter", windowsVirtualKeyCode: 13});
await sleep(800);
check(await ev(`document.querySelectorAll("g.node.server text.row").length`) === 0, "Enter closes it");
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); for (const c of clients) c.close(); process.exit(ok ? 0 : 1);
