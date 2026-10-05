// experiment: "nested boxes" -- a struct-valued member gets its own box, in a
// group outline with its holder; edges from elsewhere arrive at it
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
const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); if (r.result.exceptionDetails) console.log("EXC", JSON.stringify(r.result.exceptionDetails).slice(0, 400)); return r.result.result?.value; };
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
await call("Emulation.setDeviceMetricsOverride", {width: 1600, height: 1100, deviceScaleFactor: 1, mobile: false});
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event`)); i++) await sleep(100);
await sleep(400);
const ids = async () => ev(`[...document.querySelectorAll("g.node")].map(g => g.__data__.id).sort()`);
const G = (id) => `[...document.querySelectorAll("g.node")].find(g => g.__data__.id === ${JSON.stringify(id)})`;
const rowtext = (id, name) => ev(`(() => { const t = [...${G(id)}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === ${JSON.stringify(name)});
  return t ? [...t.querySelectorAll(":scope > tspan")].map(x => x.firstChild ? x.firstChild.nodeValue : "").join("") : null; })()`);

// Show all: every struct with members gets a box, grouped with its holder
await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
const on_ids = await ids();
const nested = on_ids.filter(x => /^(server|session:\d+)\//.test(x));
console.log("   nested boxes:", JSON.stringify(nested));
check(JSON.stringify(nested.sort()) === JSON.stringify(["server/session_table_", "server/url_router_", "server/ws_config_", "session:1/router_", "session:2/router_"]),
      "the structs get boxes -- url_router_, session_table_, ws_config_, each session's router_");
check(await ev(`[...document.querySelectorAll("g.node")].filter(g => g.__data__.kind === "nested").map(g => g.__data__.id).sort().join()`) === nested.join(),
      "... each of kind nested");
check(!(await ev(`document.getElementById("nested-boxes")`)), "no nested-boxes checkbox: always on");
// groups enclose their boxes
const enc = await ev(`(() => { const out = {}; for (const r of document.querySelectorAll("rect.group")) {
    const g = r.getBoundingClientRect(); const top = r.__data__.id.slice(4);
    const kids = [...document.querySelectorAll("g.node")].filter(n => n.__data__.id === top || n.__data__.group === top);
    out[top] = kids.length > 1 && kids.every(n => { const b = n.querySelector(":scope > rect").getBoundingClientRect();
      return b.left >= g.left - 1 && b.top >= g.top - 1 && b.right <= g.right + 1 && b.bottom <= g.bottom + 1; }); } return out; })()`);
console.log("   groups:", JSON.stringify(enc));
check(enc["server"] === true && enc["session:1"] === true, "each group outline encloses its holder and nested boxes");
// edges from elsewhere arrive at the nested box
const into = await ev(`[...document.querySelectorAll("path.edge.member")].filter(p => p.__data__.target === "server/url_router_").map(p => p.__data__.from).sort()`);
check(JSON.stringify(into) === JSON.stringify(["server", "session:1/router_", "session:2/router_"]), "UrlRouter's box: edges from the server and from both session routers: " + JSON.stringify(into));
check(await ev(`document.querySelectorAll("path.edge.link, path.edge.owns, path.edge.nests").length`) === 0, "no ownership fallback edges drawn (endpoints reached through url_router_)");
// the holder's row is a ref row; its ▾ hides the nested box
await ev(`expanded.add("server"); redraw(); 1`); await ev(`settled()`);
const r = await rowtext("server", "url_router_");
check(r && r.trim().endsWith("▾ (→)"), "server's url_router_ row: ▾ (→): " + JSON.stringify(r));
await ev(`${G("server")}.querySelector("text.row:nth-of-type(1)") && 1`);
await ev(`[...${G("server")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "url_router_").querySelector("tspan.rtoggle").dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await ev(`settled()`);
check(!(await ids()).includes("server/url_router_"), "its ▾ hides the UrlRouter box");
await ev(`[...${G("server")}.querySelectorAll("text.row")].find(t => t.__data__.m._name_ === "url_router_").querySelector("tspan.rtoggle").dispatchEvent(new MouseEvent("click", {bubbles: true}))`); await ev(`settled()`);
check((await ids()).includes("server/url_router_"), "its ▸ shows it again");
// the box triangle / menu: nested structs are children
check(await ev(`tree.kids.get("server").includes("server/url_router_")`), "nested boxes are their holder's children (triangle, menu)");
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
