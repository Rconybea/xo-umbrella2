// an endpoint's StreamReceiver drawn as its own box: owned by the endpoint,
// named by its most-derived type, reached by the receiver_ row; in the legend
import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result.result?.value; };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event && document.querySelectorAll("g.node").length >= 1 && src.link !== null`)); i++) await sleep(100);
await sleep(500);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const R = "stream:/introspect:receiver";
// the snapshot carries it; /demo (no receiver) doesn't
const recv = await ev(`member_value(last_event, "server").endpoints.map(e => [e.pattern, e.receiver && e.receiver._short_type_, e.receiver && e.receiver._canonical_type_])`);
console.log("   receivers:", JSON.stringify(recv));
check(recv.some(r => r[0] === "/introspect" && r[1] === "IntrospectReceiver" && /IntrospectReceiver$/.test(r[2])), "/introspect's receiver: IntrospectReceiver, by its own type");
check(recv.filter(r => r[0] !== "/introspect").every(r => r[1] === null), "the other endpoints: no receiver");
// not drawn by default; the endpoint's triangle shows it
await ev(`show_box("stream:/introspect"); 1`); await ev(`settled()`);
check(!(await ev(`!!${G(R)}`)), "hidden by default (a child of its endpoint)");
await ev(`toggle_children("stream:/introspect"); 1`); await ev(`settled()`);
check(await ev(`!!${G(R)}`), "the endpoint's triangle shows it");
check(await ev(`${G(R)}.querySelector("text.label").textContent`) === "IntrospectReceiver", "labelled by its short type");
// the endpoint's receiver_ row: a ▾ (→) and the edge
await ev(`expanded.add("stream:/introspect"); redraw(); 1`); await ev(`settled()`);
const row = await ev(`(() => { const t = [...${G("stream:/introspect")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "receiver_");
  return [...t.querySelectorAll(":scope > tspan")].map(x => x.firstChild ? x.firstChild.nodeValue : "").join(""); })()`);
check(row.trim().endsWith("▾ (→)"), "receiver_ row: ▾ (→): " + JSON.stringify(row));
check(await ev(`[...document.querySelectorAll("path.edge.member")].some(p => p.__data__.target === ${JSON.stringify(R)})`), "an edge from the endpoint to it");
// colour + legend
const fill = await ev(`getComputedStyle(${G(R)}.querySelector(":scope > rect")).fill`);
const leg = await ev(`[...document.querySelectorAll("g.legend g.entry")].map(e => [e.querySelector("text").textContent, getComputedStyle(e.querySelector("rect")).fill])`);
console.log("   legend:", JSON.stringify(leg.map(x => x[0])));
check(leg.some(([t, f]) => t === "IntrospectReceiver" && f === fill) && fill === "rgb(226, 244, 243)", "pale teal, and in the legend: " + fill);
// its source: the menu's Open source has a link
await ev(`${G(R)}.dispatchEvent(new MouseEvent("contextmenu", {bubbles: true, cancelable: true, clientX: 300, clientY: 300}))`); await sleep(200);
const items = await ev(`[...document.querySelectorAll("#ctxmenu button")].map(b => b.textContent + (b.disabled ? " (disabled: " + b.title + ")" : ""))`);
console.log("   menu:", JSON.stringify(items));
check(items.includes("Open source"), "Open source enabled: its exact type is in the source map");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); process.exit(ok ? 0 : 1);
