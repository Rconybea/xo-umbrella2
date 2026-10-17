import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const cl = new WebSocket(`ws://localhost:${port}/`, "lws-minimal"); await new Promise(r => cl.onopen = r); cl.send('{"cmd":"subscribe","stream":"/demo/1"}'); await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => (await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true})).result.result?.value;
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok = " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1600, height: 1100, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event`)); i++) await sleep(100);
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
// each box: line 1 its short type; line 2 its title, where that says more
const hd = await ev(`Object.fromEntries([...document.querySelectorAll("g.node")].map(g => { const l = g.querySelector(":scope > text.label"), s = g.querySelector(":scope > text.sub");
  return [g.__data__.id, {type: l.textContent, sub: s.getAttribute("display") === "none" ? null : s.textContent, short: g.__data__.obj && g.__data__.obj._short_type_}]; }))`);
console.log("   headers:", JSON.stringify(Object.entries(hd).map(([k, v]) => [k, v.type, v.sub])));
check(Object.values(hd).every(h => h.type === h.short), "line 1: every box's short type");
const lport = await ev(`member_value(member_value(last_event, "server"), "listen_port")`);
check(hd.server.sub === `:${lport} (running)`, "server: :port (running): " + hd.server.sub);
check(hd["http:/types"].sub === "/types" && hd["stream:/introspect"].sub === "/introspect", "endpoints: their pattern");
check(hd["session:1"].sub === "session 1", "session: session 1");
check(hd["session:1:sub:0"].sub.startsWith("sub 0 · /"), "subscription: sub 0 · <stream>: " + hd["session:1:sub:0"].sub);
check(["session:1:sender", "session:1:sub:0:sink", "stream:/introspect:receiver", "server/url_router", "session:1/router"].every(id => hd[id].sub === null),
      "sender, sink, receiver, nested: the type alone");
// the title: smaller, not grey -- the type's colour
const st = await ev(`(() => { const f = (id, c) => getComputedStyle([...document.querySelectorAll("g.node")].find(g => g.__data__.id === id).querySelector(":scope > text." + c));
  return {big: [f("server", "label").fontSize, f("server", "sub").fontSize], small: [f("session:1:sub:0", "label").fontSize, f("session:1:sub:0", "sub").fontSize],
          fill: [f("server", "label").fill, f("server", "sub").fill]}; })()`);
check(st.big[0] === "14px" && st.big[1] === "12px" && st.small[0] === "12px" && st.small[1] === "11px", "title one size down: " + JSON.stringify(st));
check(st.fill[0] === st.fill[1], "title in the type's colour: " + JSON.stringify(st.fill));
// the menu button and the children triangle: level with the type's line
const lv = await ev(`["server", "session:1", "session:1:sub:0"].map(id => { const g = [...document.querySelectorAll("g.node")].find(g => g.__data__.id === id);
  const c = e => { const r = e.getBoundingClientRect(); return r.top + r.height / 2; };
  const l = g.querySelector(":scope > text.label").getBoundingClientRect(), s = g.querySelector(":scope > text.sub").getBoundingClientRect();
  return {id, mbtn: Math.abs(c(g.querySelector(":scope > g.mbtn > rect")) - c(g.querySelector(":scope > text.label"))), kids: Math.abs(c(g.querySelector(":scope > g.kbtn > text.kids")) - c(g.querySelector(":scope > text.label"))), below: s.top >= l.bottom - 3}; })`);
console.log("   level:", JSON.stringify(lv));
check(lv.every(x => x.mbtn < 4 && x.kids < 4 && x.below), "menu button and triangle level with the type; the title below it");
// the children triangle: inside the box, first on the type's line; the type
// follows it.  A box with no children has none, and no gap
const tri = await ev(`[...document.querySelectorAll("g.node")].map(g => { const b = g.querySelector(":scope > rect").getBoundingClientRect();
  const k = g.querySelector(":scope > g.kbtn"), l = g.querySelector(":scope > text.label");
  const kr = k.getBoundingClientRect(), lr = l.getBoundingClientRect();
  return {id: g.__data__.id, n: g.__data__.n_children, shown: getComputedStyle(k).display !== "none",
          inside: kr.left >= b.left && kr.right <= b.right && kr.top >= b.top && kr.bottom <= b.bottom,
          before: kr.right <= lr.left + 1, label_x: +l.getAttribute("x")}; })`);
const with_kids = tri.filter(t => t.n > 0), without = tri.filter(t => t.n === 0);
console.log("   triangles:", JSON.stringify(with_kids.map(t => [t.id, t.inside, t.before, t.label_x])));
check(with_kids.length > 0 && with_kids.every(t => t.shown && t.inside && t.before && t.label_x > 12),
      "children triangle: inside its box, before the type, which moves right");
check(without.length > 0 && without.every(t => !t.shown && t.label_x === 12), "no children: no triangle, type at the margin");
// hovered, its square shows -- as the menu button's does: a control.  On a
// box whose button nothing covers (the legend may sit over some)
const kb = await ev(`(() => { for (const g of document.querySelectorAll("g.node")) {
    const k = g.querySelector(":scope > g.kbtn"); if (!g.__data__.n_children) continue;
    const r = k.querySelector(":scope > rect").getBoundingClientRect(), x = r.left + r.width / 2, y = r.top + r.height / 2;
    if (k.contains(document.elementFromPoint(x, y))) return {id: g.__data__.id, x, y}; }
  return null; })()`);
check(kb !== null, "a box with children whose button is uncovered: " + (kb && kb.id));
const kfill = () => ev(`getComputedStyle([...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(kb.id)}).querySelector(":scope > g.kbtn > rect")).fillOpacity`);
check(await kfill() === "0", "triangle's square: clear until hovered");
await call("Input.dispatchMouseEvent", {type: "mouseMoved", x: kb.x, y: kb.y});
await sleep(100);
check(await kfill() === "0.85", "hovered: its square shows, as the menu button's does: " + await kfill());
check(await ev(`getComputedStyle([...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(kb.id)}).querySelector(":scope > g.kbtn")).cursor`) === "pointer",
      "and a pointer cursor");
await call("Input.dispatchMouseEvent", {type: "mouseMoved", x: 5, y: 5});
// the menu button: in the box's top-right corner -- whatever the box's width
const corner = async () => ev(`(() => { const k = d3.zoomTransform(graph_svg.node()).k;
  return [...document.querySelectorAll("g.node")].map(g => { const b = g.querySelector(":scope > rect").getBoundingClientRect(), m = g.querySelector(":scope > g.mbtn > rect").getBoundingClientRect();
    return {id: g.__data__.id, gap: Math.round((b.right - m.right) / k * 10) / 10}; }); })()`);
const cn = await corner();
console.log("   corner gaps:", JSON.stringify(cn.map(c => c.gap)));
check(cn.every(c => Math.abs(c.gap - 12) < 1), "menu button: top-right corner of every box (12 from its right edge)");
// rows start below the title
await ev(`expanded.add("session:1"); redraw(); 1`); await ev(`settled()`);
// expanded, a box can grow wider than its header: the button follows the corner
{ const w0 = await ev(`Object.fromEntries([...document.querySelectorAll("g.node")].map(g => [g.__data__.id, g.__data__.w]))`);
  await ev(`for (const g of document.querySelectorAll("g.node")) if (g.__data__.expandable) expanded.add(g.__data__.id); redraw(); 1`); await ev(`settled()`);
  const w1 = await ev(`Object.fromEntries([...document.querySelectorAll("g.node")].map(g => [g.__data__.id, g.__data__.w]))`);
  const grew = Object.keys(w1).filter(id => w0[id] !== undefined && w1[id] > w0[id] + 20);
  check(grew.length > 0, "some box grows wider when expanded: " + JSON.stringify(grew));
  const c = (await corner()).filter(c => grew.includes(c.id));
  console.log("   grown boxes' corner gaps:", JSON.stringify(c));
  check(c.every(c => Math.abs(c.gap - 12) < 1), "expanded, wider: the menu button stays in the corner");
  await ev(`expanded.clear(); expanded.add("session:1"); redraw(); 1`); await ev(`settled()`); }
check(await ev(`(() => { const g = [...document.querySelectorAll("g.node")].find(g => g.__data__.id === "session:1");
  return g.querySelector("text.row").getBoundingClientRect().top >= g.querySelector(":scope > text.sub").getBoundingClientRect().bottom - 1; })()`), "an open box's rows start below its title");
// menus still name a box by its title
await ev(`[...document.querySelectorAll("g.node")].find(g => g.__data__.id === "session:1").dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 0, clientY: 0}))`); await sleep(200);
check((await ev(`[...document.querySelectorAll("#ctxmenu button")].map(b => b.textContent)`)).includes("Hide session 1"), "menu: Hide session 1");
await ev(`document.body.dispatchEvent(new KeyboardEvent("keydown", {key: "Escape", bubbles: true}))`);
await ev(`document.getElementById("fit").click()`); await sleep(700);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
