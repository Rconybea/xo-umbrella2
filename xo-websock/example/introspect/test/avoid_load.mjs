// libavoid loading (.xo-backlog/introspect-layout/spec.md, "Failure"):
// blocked -> a visible error and no drawing; slow -> the first draw waits
import fs from "node:fs";
const [,, cdp_port, port, out] = process.argv;
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
let ok = true; const check = (c, msg) => { console.log(`${c ? "ok  " : "FAIL"} ${msg}`); if (!c) ok = false; };
async function page(mode) {   // mode: "block" | "delay"
  const tgt = await (await fetch(`http://localhost:${cdp_port}/json/new?about:blank`, {method: "PUT"})).json();
  const ws = new WebSocket(tgt.webSocketDebuggerUrl); await new Promise(r => ws.onopen = r);
  let seq = 0; const pending = new Map();
  const call = (method, params = {}) => new Promise(r => { const id = ++seq; pending.set(id, r); ws.send(JSON.stringify({id, method, params})); });
  ws.onmessage = async (e) => { const m = JSON.parse(e.data);
    if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); return; }
    if (m.method === "Fetch.requestPaused") {
      const rid = m.params.requestId;
      if (mode === "block") call("Fetch.failRequest", {requestId: rid, errorReason: "BlockedByClient"});
      else { await sleep(3000); call("Fetch.continueRequest", {requestId: rid}); } } };
  const ev = async (expr) => { const r = await call("Runtime.evaluate", {expression: expr, returnByValue: true, awaitPromise: true}); return r.result.result?.value; };
  await call("Fetch.enable", {patterns: [{urlPattern: "*libavoid*"}]});
  await call("Page.navigate", {url: `http://localhost:${port}/`});
  return {ws, call, ev};
}
{ // blocked
  const {ws, ev} = await page("block");
  let st = "";
  for (let i = 0; i < 100 && !/libavoid/.test(st); i++) { await sleep(100); st = await ev(`document.getElementById("status").textContent`) || ""; }
  check(/^error: edge router \(libavoid\) failed to load/.test(st), "blocked: the status line says so: " + JSON.stringify(st));
  check(await ev(`document.querySelectorAll("g.node").length`) === 0, "blocked: nothing drawn");
  ws.close();
}
{ // [RF 1] slow: the snapshot arrives first; the draw waits, then routes
  const {ws, call, ev} = await page("delay");
  for (let i = 0; i < 150 && !(await ev(`typeof last_event !== "undefined" && !!last_event && document.querySelectorAll("g.node").length >= 1`)); i++) await sleep(100);
  await ev(`settled()`);
  await ev(`document.getElementById("show-all").click(); 1`); await ev(`settled()`);
  check(!/error/.test(await ev(`document.getElementById("status").textContent`)), "slow: no error");
  check(await ev(`document.querySelectorAll("path.edge.member").length`) >= 10, "slow: member edges drawn");
  fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
  ws.close();
}
console.log(ok ? "ALL OK" : "SOME FAILED");
process.exit(ok ? 0 : 1);
