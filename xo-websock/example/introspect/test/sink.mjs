// a subscription's WebsocketSink drawn as its own box: owned by the
// subscription, reached by its sink_ row; opens to its members (sender_ ->
// the session's sender box); in the legend
import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const cl = new WebSocket(`ws://localhost:${port}/`, "lws-minimal");
await new Promise(r => cl.onopen = r); cl.send('{"cmd":"subscribe","stream":"/demo/1"}'); await sleep(300);
const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?http://localhost:${port}/`, {method: "PUT"})).json();
const ws = new WebSocket(tgt.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
let seq = 0; const pending = new Map();
ws.onmessage = (ev) => { const m = JSON.parse(ev.data); if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); } };
const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result.result?.value; };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1500, height: 1000, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
await sleep(500);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const rowtext = (id, name) => ev(`(() => { const t = [...${G(id)}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === ${JSON.stringify(name)});
  return t ? [...t.querySelectorAll(":scope > tspan")].map(x => x.firstChild ? x.firstChild.nodeValue : "").join("") : null; })()`);
const SUB = "session:1:sub:0", K = "session:1:sub:0:sink";
// the snapshot: the sink, with its members
const mem = await ev(`member_value(last_event, "server").sessions[0].subscriptions[0].sink._members_.map(m => m._name_)`);
// reflected members first (stream_name_, sub_id_, n_in_ev_), then the two
// printed elsewhere, as refs (xo-printjson#06)
check(JSON.stringify(mem) === JSON.stringify(["stream_name_", "sub_id_", "n_in_ev_", "sender_", "pjson_"]), "the sink's printer lists its members: " + JSON.stringify(mem));
// hidden by default; the subscription's triangle shows it
await ev(`show_box(${JSON.stringify(SUB)}); 1`); await ev(`settled()`);
check(!(await ev(`!!${G(K)}`)), "hidden by default (a child of its subscription)");
await ev(`toggle_children(${JSON.stringify(SUB)}); 1`); await ev(`settled()`);
check(await ev(`!!${G(K)}`) && await ev(`${G(K)}.querySelector("text.label").textContent`) === "WebsocketSinkImpl" && await ev(`${G(K)}.querySelector("text.sub").getAttribute("display")`) === "none", "the subscription's triangle shows it, headed by its type alone (WebsocketSinkImpl)");
// the subscription's sink_ row: no longer "not drawn"
await ev(`expanded.add(${JSON.stringify(SUB)}); redraw(); 1`); await ev(`settled()`);
const r = await rowtext(SUB, "sink_");
check(r && r.trim().endsWith("▾ (→)"), "sink_ row: ▾ (→): " + JSON.stringify(r));
check(await ev(`[...document.querySelectorAll("path.edge.member")].some(p => p.__data__.target === ${JSON.stringify(K)})`), "an edge from the subscription to it");
// the sink box opens: its members; sender_ refers to the session's sender
check(await ev(`${G(K)}.classList.contains("expandable")`), "the sink box opens");
await ev(`expanded.add(${JSON.stringify(K)}); show_box("session:1:sender"); redraw(); 1`); await ev(`settled()`);
const s1 = await rowtext(K, "sender_"), s2 = await rowtext(K, "stream_name_");
check(s1 && s1.trim().endsWith("▾ (→)") && s2 && s2.includes('"/demo/1"'), "its rows: sender_ ▾ (→), stream_name_ /demo/1: " + JSON.stringify([s1, s2]));
check(await ev(`[...document.querySelectorAll("path.edge.member")].some(p => p.__data__.from === ${JSON.stringify(K)} && p.__data__.target === "session:1:sender")`), "an edge from the sink to its session's sender");
// colour + legend
const fill = await ev(`getComputedStyle(${G(K)}.querySelector(":scope > rect")).fill`);
const leg = await ev(`[...document.querySelectorAll("g.legend g.entry")].map(e => [e.querySelector("text").textContent, getComputedStyle(e.querySelector("rect")).fill])`);
console.log("   legend:", JSON.stringify(leg.map(x => x[0])));
check(fill === "rgb(237, 240, 245)" && leg.some(([t, f]) => t === "WebsocketSinkImpl" && f === fill), "pale slate, in the legend as WebsocketSinkImpl: " + fill);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
