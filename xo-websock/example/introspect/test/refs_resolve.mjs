// every {"_ref_": n} in an introspect snapshot names an "_id_" in the same snapshot
const [,, cdp_port, port] = process.argv;
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
const ev = async (expr) => (await call("Runtime.evaluate", {expression: expr, returnByValue: true})).result.result?.value;
for (let i = 0; i < 100 && !(await ev(`typeof last_event !== "undefined" && !!last_event`)); i++) await sleep(100);
const r = await ev(`(() => {
  const ids = new Map(), refs = [];
  const walk = (v, path) => {
    if (Array.isArray(v)) { v.forEach((x, i) => walk(x, path + "[" + i + "]")); return; }
    if (v && typeof v === "object") {
      if (typeof v._id_ === "number") ids.set(v._id_, (ids.get(v._id_) || []).concat([path + " " + v._name_]));
      if ("_ref_" in v) refs.push([v._ref_, path]);
      for (const k of Object.keys(v)) walk(v[k], path + "." + k);
    }
  };
  walk(last_event, "$");
  const dup = [...ids].filter(([k, v]) => v.length > 1);
  return {n_ids: ids.size, n_refs: refs.length, dangling: refs.filter(([n]) => !ids.has(n)).map(([n, p]) => n + " at " + p), dup};
})()`);
console.log(JSON.stringify(r, null, 1));
const ok = r.dangling.length === 0 && r.dup.length === 0;
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
