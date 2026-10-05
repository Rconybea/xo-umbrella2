import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const cl = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
await new Promise(r => cl.onopen = r); cl.send('{"cmd":"subscribe","stream":"/demo/1"}');
await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl);
await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 400)); return r.result.result?.value; };
const mouse = async (type, x, y, button) => call("Input.dispatchMouseEvent", {type, x, y, button, clickCount: 1});
const click_at = async (p) => { await mouse("mouseMoved", p.x, p.y, "none"); await mouse("mousePressed", p.x, p.y, "left"); await mouse("mouseReleased", p.x, p.y, "left"); };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok = " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`document.querySelectorAll("g.node").length >= 1 && !!last_event`)); i++) await sleep(100);
await sleep(600);
const ids = async () => ev(`[...document.querySelectorAll("g.node")].map(g => g.__data__.id).sort()`);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const kids_text = async (id) => ev(`${G(id)}.querySelector("text.kids").firstChild.nodeValue`);
const center = async (sel) => ev(`(() => { const t = ${sel}.getBoundingClientRect(); return {x: t.left + t.width / 2, y: t.top + t.height / 2}; })()`);
const rowtext = `t => [...t.querySelectorAll(":scope > tspan")].map(ts => ts.firstChild.nodeValue).join("")`;
const rows = async (id) => ev(`[...${G(id)}.querySelectorAll("text.row")].map(${rowtext})`);
const row_sel = (id, prefix) => `[...${G(id)}.querySelectorAll("text.row")].find(t => (${rowtext})(t).trimStart().startsWith(${JSON.stringify(prefix)}))`;
const click_row = async (id, prefix) => ev(`${row_sel(id, prefix)}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`);
const menu = async (id) => { await ev(`${G(id)}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 0, clientY: 0}))`); await sleep(200);
  return ev(`[...document.querySelectorAll("#ctxmenu button")].map(b => b.textContent)`); };
const menu_click = async (label) => { await ev(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.textContent === ${JSON.stringify(label)}).click()`); await sleep(800); };
const esc = () => ev(`document.body.dispatchEvent(new KeyboardEvent("keydown", {key: "Escape", bubbles: true}))`);
// nested structs have boxes of their own, and own what their refs reach:
// the server owns ws_config_, url_router_, session_table_; the UrlRouter its
// endpoints; the session table its sessions; a session's router_ its sub
const U = "server/url_router_", T = "server/session_table_", R1 = "session:1/router_";

check(JSON.stringify(await ids()) === JSON.stringify(["server"]), "default: only the Webserver box");
check(await kids_text("server") === "▸3", "its toggle: 3 hidden children (its nested boxes): " + await kids_text("server"));
let sm = await menu("server");
console.log(" =  server menu:", JSON.stringify(sm));
check(sm.includes("Show children (+3)") && !sm.some(t => t.startsWith("Hide children")), "nothing drawn: Show children (+3), no Hide children");
check(["Show ▸ WebserverConfig", "Show ▸ UrlRouter"].every(t => sm.includes(t)) && !sm.some(t => t.startsWith("Show ▸ /")), "Show ▸ per nested box; the endpoints are the UrlRouter's");
check(sm.some(t => /^Hide Webserver :\d+ \(running\)$/.test(t)), "Hide <its label>");
check(await ev(`[...document.querySelectorAll("#ctxmenu button")].find(b => b.textContent.startsWith("Hide Webserver")).disabled`) === true, "... disabled on the Webserver");
await esc();

// the server's url_router_ row: ▸ (→); its ▸ draws the UrlRouter box
await ev(`${G("server")}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(700);
check((await rows("server")).some(t => t.startsWith("url_router_:") && t.endsWith(": ▸ (→)")), "url_router_ row: ▸ (→), its box hidden");
await click_at(await center(`${row_sel("server", "url_router_:")}.querySelector("tspan.rtoggle")`)); await sleep(900);
check(JSON.stringify(await ids()) === JSON.stringify(["server", U]), "its ▸ draws the UrlRouter box: " + JSON.stringify(await ids()));
check(await kids_text(U) === "▸4", "UrlRouter's toggle: its 4 endpoints hidden: " + await kids_text(U));
// open url_router_'s stream_map_; its refs carry a ▸
await ev(`${G(U)}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(700);
await click_row(U, "stream_map_:"); await sleep(700);
let r = await rows(U);
console.log(" =  rows:", JSON.stringify(r.filter(t => t.includes("→"))));
const intro = r.find(t => t.includes('"/introspect"'));
check(intro && intro.trim().endsWith(": ▸ (→)"), "stream_map_ /introspect: ▸ (→), its box hidden: " + intro);

// a real click on that ▸: shows that endpoint alone, edge drawn
const pre = intro.trim().slice(0, 8);
await click_at(await center(`${row_sel(U, pre)}.querySelector("tspan.rtoggle")`)); await sleep(900);
let v = await ids();
check(v.length === 3 && v.some(x => x.startsWith("stream:/introspect")), "▸ shows just that endpoint: " + JSON.stringify(v));
const intro2 = (await rows(U)).find(t => t.trim().startsWith(pre));
check(intro2 && intro2.trim().endsWith(": ▾ (→)"), "now ▾ (→): " + intro2);
check(await ev(`document.querySelectorAll("path.edge.member").length`) === 2, "two member edges: server -> UrlRouter -> /introspect");
check(await kids_text(U) === "▸3", "UrlRouter toggle: 3 still hidden: " + await kids_text(U));
// ▾ hides it again
await click_at(await center(`${row_sel(U, pre)}.querySelector("tspan.rtoggle")`)); await sleep(900);
check(JSON.stringify(await ids()) === JSON.stringify(["server", U]), "▾ hides it again");

// RC's case: the UrlRouter with only /introspect drawn -- both counts
await ev(`show_box("stream:/introspect"); 1`); await sleep(900);
sm = await menu(U);
check(sm.includes("Show children (+3)") && sm.includes("Hide children (-1)"), "partly shown: Show children (+3) and Hide children (-1): " + JSON.stringify(sm.filter(t => t.includes("children"))));
await menu_click("Hide children (-1)");
check(JSON.stringify(await ids()) === JSON.stringify(["server", U]), "Hide children (-1) hides /introspect");
await ev(`show_box("stream:/introspect"); 1`); await sleep(900);
await menu(U); await menu_click("Show children (+3)");
check((await ids()).length === 6, "Show children (+3) draws the other 3: " + (await ids()).length);
sm = await menu(U);
check(sm.includes("Hide children (-4)") && !sm.some(t => t.startsWith("Show children")), "all drawn: Hide children (-4), no Show children");
await menu_click("Hide children (-4)");
check(JSON.stringify(await ids()) === JSON.stringify(["server", U]), "Hide children (-4): back to the server and its UrlRouter");
// the server's Hide children hides its nested box -- and what hangs off it
await ev(`show_box("stream:/demo/"); 1`); await sleep(900);
sm = await menu("server");
check(sm.includes("Show children (+2)") && sm.includes("Hide children (-1)"), "server: Show children (+2), Hide children (-1): " + JSON.stringify(sm.filter(t => t.includes("children"))));
await menu_click("Hide children (-1)");
check(JSON.stringify(await ids()) === JSON.stringify(["server"]), "server Hide children (-1): the UrlRouter and /demo/ go");

const set_open = async (id, on) => { if ((await ev(`expanded.has(${JSON.stringify(id)})`)) !== on) { await ev(`${G(id)}.dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(900); } };
// a deep box: the path to it, nothing beside it
await ev(`show_box("session:1:sub:0"); 1`); await sleep(900);
v = await ids();
check(JSON.stringify(v) === JSON.stringify(["server", T, "session:1", R1, "session:1:sub:0"].sort()), "show sub: the path to it (session table, session, its router), not the sender, not session:2: " + JSON.stringify(v));
let items = await menu("session:1");
console.log(" =  session:1 menu:", JSON.stringify(items));
const show_sender = items.find(t => t.startsWith("Show ▸ "));
check(items.includes("Hide session 1") && show_sender === "Show ▸ sender" && items.includes("Hide ▸ WsSessionRouter"), "session menu: Hide session 1, Show ▸ sender, Hide ▸ its router");
await menu_click(show_sender);
check((await ids()).includes("session:1:sender"), "Show ▸ shows the sender");
items = await menu("session:1");
check(items.includes("Hide ▸ sender") && items.includes("Hide ▸ WsSessionRouter") && !items.some(t => t.startsWith("Show ▸ ")),
      "both children drawn: a Hide ▸ for each, no Show ▸");
await menu_click("Hide ▸ sender");
check(!(await ids()).includes("session:1:sender") && (await ids()).includes("session:1:sub:0"), "Hide ▸ sender hides the sender, not the sub");
items = await menu("session:1");
check(items.includes("Show ▸ sender"), "... and its entry flips to Show ▸ sender");
await esc();
await menu("session:1"); await menu_click("Hide session 1");
check(JSON.stringify(await ids()) === JSON.stringify(["server", T]), "Hide session:1: what hung off it goes too");
await ev(`show_box("session:1"); 1`); await sleep(900);
v = await ids();
check(!v.includes("session:1:sender") && v.includes("session:1:sub:0"), "show session:1 again: its sub comes back, not the sender (Hide ▸ sender dropped it): " + JSON.stringify(v));
await ev(`document.getElementById("hide-all").click()`); await sleep(800);

// the box triangle on a COLLAPSED server: edges drawn as the refs would be
await ev(`expanded.delete("server"); redraw(); 1`); await sleep(800);
check(!(await ev(`expanded.has("server")`)), "server collapsed");
await click_at(await center(`${G("server")}.querySelector("text.kids")`)); await sleep(900);
check((await ids()).length === 4, "triangle: its 3 nested boxes shown");
check(await kids_text("server") === "▾", "toggle ▾");
check(await ev(`document.querySelectorAll("path.edge.member").length`) === 3
      && await ev(`document.querySelectorAll("path.edge.link, path.edge.owns, path.edge.nests").length`) === 0,
      "3 member edges from the collapsed server, no ownership edges");
const tips = await ev(`[...document.querySelectorAll("path.edge.member > title")].map(t => t.textContent).sort()`);
console.log(" =  tooltips:", JSON.stringify(tips));
check(["ws_config_", "url_router_", "session_table_"].every(n => tips.some(t => t.endsWith(` · ${n} (includes)`))), "each edge's tooltip names its member, and its kind");
await click_at(await center(`${G("server")}.querySelector("text.kids")`)); await sleep(900);
check(JSON.stringify(await ids()) === JSON.stringify(["server"]), "triangle again: hidden");

// /types alone; then a session alone -- neither drags in the other
const types_id = await ev(`layout(last_event).nodes.map(d => d.id).find(x => x.startsWith("http:/types"))`);
await ev(`show_box(${JSON.stringify(types_id)}); 1`); await sleep(900);
check(JSON.stringify(await ids()) === JSON.stringify([types_id, "server", U].sort()), "/types alone (by way of the UrlRouter): " + JSON.stringify(await ids()));
await ev(`document.getElementById("hide-all").click()`); await sleep(800);
await ev(`show_box("session:1"); 1`); await sleep(900);
check(JSON.stringify(await ids()) === JSON.stringify(["server", T, "session:1"]), "session:1 alone (by way of the session table): " + JSON.stringify(await ids()));
await ev(`document.getElementById("hide-all").click()`); await sleep(800);

// collapsing a box leaves what is drawn alone (it used to drop the box's edges)
await ev(`show_box(${JSON.stringify(U)}); 1`); await sleep(900);
await set_open(U, true);
await ev(`expanded.add("${U}/stream_map_"); redraw(); 1`); await sleep(800);
await click_at(await center(`${row_sel(U, '["/demo/"]')}.querySelector("tspan.rtoggle")`)); await sleep(900);
await click_at(await center(`${row_sel(U, '["/introspect"]')}.querySelector("tspan.rtoggle")`)); await sleep(900);
await click_row(U, "stream_map_:"); await sleep(800);   // a member row closing keeps the edges
v = await ids();
check(v.length === 4 && await ev(`[...document.querySelectorAll("path.edge.member")].filter(p => p.__data__.from === "${U}").length`) === 2,
      "stream_map_ closed: both endpoints and both edges stay: " + JSON.stringify(v));
await ev(`show_box("session:1:sub:0"); 1`); await sleep(900);
await set_open("session:1:sub:0", true);
// a second wanted edge into /demo/ (already drawn): set directly -- the row's
// triangle follows its target, so clicking ▾ here would hide /demo/
await ev(`wanted.add("session:1:sub:0/endpoint_"); redraw(); 1`); await sleep(900);
check(await ev(`wanted.has("session:1:sub:0/endpoint_")`), "sub's endpoint_ edge wanted");
// collapse the UrlRouter: nothing drawn changes -- its edges still leave it
const before = await ids();
const edges_before = await ev(`document.querySelectorAll("path.edge.member").length`);
await set_open(U, false);
check(JSON.stringify(await ids()) === JSON.stringify(before), "UrlRouter collapsed: the same boxes drawn: " + JSON.stringify(await ids()));
check(await ev(`document.querySelectorAll("path.edge.member").length`) === edges_before
      && await ev(`[...document.querySelectorAll("path.edge.member")].filter(p => p.__data__.from === "${U}").length`) === 2,
      "... the same member edges, the UrlRouter's from its collapsed box");
check(await ev(`wanted.has('${U}/stream_map_/["/introspect"]')`), "... its edges still wanted");
// collapse a sub: its endpoint_ edge stays wanted
await set_open("session:1:sub:0", true);
await set_open("session:1:sub:0", false);
check(await ev(`wanted.has("session:1:sub:0/endpoint_")`) && JSON.stringify(await ids()) === JSON.stringify(before),
      "sub collapsed: its edge still wanted, nothing drawn changes");

// Hide ▸ child hides the child even when another wanted edge reaches it
await ev(`document.getElementById("hide-all").click()`); await sleep(800);
await ev(`show_box("stream:/demo/"); show_box("session:1:sub:0"); wanted.add("session:1:sub:0/endpoint_"); redraw(); 1`); await sleep(900);
check((await ids()).includes("stream:/demo/"), "/demo/ reached by the UrlRouter's edge and by the sub's");
items = await menu(U);
const hide_demo = items.find(t => t.startsWith("Hide ▸ /demo/"));
check(!!hide_demo, "UrlRouter menu: " + hide_demo);
await menu_click(hide_demo);
check(!(await ids()).includes("stream:/demo/") && (await ids()).includes("session:1:sub:0"), "Hide ▸ /demo/ hides it despite the sub's edge; the sub stays");

// Show all / Hide all
await ev(`document.getElementById("show-all").click()`); await sleep(900);
check((await ids()).length === 19, "Show all: 19 boxes (receiver, sinks, nested structs too): " + (await ids()).length);
check(await ev(`document.querySelectorAll("path.edge.link, path.edge.owns, path.edge.nests").length`) === 0, "no fallback ownership edges needed");
const overl = await ev(`(() => { const r = [...document.querySelectorAll("g.node > rect")].map(e => e.getBoundingClientRect()); let n = 0;
  for (let i = 0; i < r.length; i++) for (let j = i + 1; j < r.length; j++) { const a = r[i], b = r[j];
    if (a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom) n++; } return n; })()`);
check(overl === 0, "no overlaps");

// a ref row's triangle follows its TARGET: the sender is reached by session
// 1's sender_, its router's sender_ and its sink's sender_ (all wanted)
await set_open("session:1", true);
const sender_row = async () => ev(`[...${G("session:1")}.querySelectorAll("text.row")].filter(t => t.__data__.m._name_ === "sender_").map(${rowtext})[0]`);
check((await sender_row()).endsWith(": ▾ (→)"), "sender_ reads ▾: its target drawn");
await ev(`[...${G("session:1")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "sender_").querySelector("tspan.rtoggle").dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(900);
check(!(await ids()).includes("session:1:sender") && (await sender_row()).endsWith(": ▸ (→)"),
      "clicking it hides the sender despite the router's and sink's edges, and it reads ▸: " + await sender_row());
await ev(`[...${G("session:1")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "sender_").querySelector("tspan.rtoggle").dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await sleep(900);
check((await ids()).includes("session:1:sender") && (await sender_row()).endsWith(": ▾ (→)"), "clicking ▸ shows it again, ▾");
await set_open("session:1", false);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png", captureBeyondViewport: true, clip: {x: 0, y: 0, width: 1800, height: 1100, scale: 1}})).result.data, "base64"));
await ev(`document.getElementById("hide-all").click()`); await sleep(800);
check(JSON.stringify(await ids()) === JSON.stringify(["server"]), "Hide all: only the Webserver");
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
