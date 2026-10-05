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
// each drawn member edge's kind, by the declared type of what it stands for
const kinds = await ev(`[...document.querySelectorAll("path.edge.member")].map(p => [p.__data__.labels.join(","), p.__data__.ref_kind, [...p.classList].find(c => c.startsWith("from-"))])`);
console.log("   edges:", JSON.stringify(kinds));
// (the server's url_router_ here; the routers' are checked below)
const want = {ws_config_: "includes", url_router_: "includes", session_table_: "includes", router_: "includes",
              'session_map_["1"]': "owns", 'session_map_["2"]': "owns", "subscription_v_[0]": "owns",
              'http_map_["/hello/"]': "shares", 'stream_map_["/introspect"]': "shares", receiver_: "shares",
              sender_: "shares", endpoint_: "shares", sink_: "shares"};
const by_label = new Map(kinds.filter(([l, k]) => !(l === "url_router_" && k === "refers")).map(([l, k]) => [l, k]));
for (const [l, k] of Object.entries(want))
    check(by_label.get(l) === k, `${l}: ${k} (${by_label.get(l)})`);
check(kinds.every(([l, k, c]) => c === `from-${k}`), "each edge's class carries its kind");
// the sender's target_ (WebserverImpl*) -- into the Webserver, never drawn: its showable edge
check(await ev(`showable.filter(e => e.key.endsWith("/target_")).every(e => e.ref_kind === "refers") && showable.some(e => e.key.endsWith("/target_"))`), "target_ (WebserverImpl*): refers");
// the router's url_router_ is a const UrlRouter&: refers, no start marker
const rr = await ev(`[...document.querySelectorAll("path.edge.member")].filter(p => p.__data__.from.endsWith("/router_") && p.__data__.target === "server/url_router_").map(p => [p.__data__.ref_kind, getComputedStyle(p).markerStart])`);
check(rr.length === 2 && rr.every(([k, m]) => k === "refers" && m === "none"), "router_.url_router_ (const UrlRouter&): refers, no start marker: " + JSON.stringify(rr));
// the markers as drawn
const ms = await ev(`Object.fromEntries(["includes", "owns", "shares"].map(k => { const p = document.querySelector("path.edge.member.from-" + k); return [k, p ? getComputedStyle(p).markerStart : null]; }))`);
check(ms.includes === 'url("#start-includes")' && ms.owns === 'url("#start-owns")' && ms.shares === 'url("#start-shares")', "start markers by kind: " + JSON.stringify(ms));
// colour by kind: includes black .. refers light grey; the arrowhead and
// exit marker in the edge's colour
const grey = {includes: "rgb(26, 26, 26)", owns: "rgb(85, 85, 85)", shares: "rgb(140, 140, 140)", refers: "rgb(196, 196, 196)"};
const col = await ev(`Object.fromEntries(["includes", "owns", "shares", "refers"].map(k => { const p = document.querySelector("path.edge.member.from-" + k);
  const ms = getComputedStyle(p).markerStart, me = getComputedStyle(p).markerEnd;
  const fill = id => getComputedStyle(document.querySelector(id + " > path")).fill;
  return [k, {stroke: getComputedStyle(p).stroke, end: me, end_fill: fill(me.slice(5, -2)), start_fill: ms === "none" ? null : fill(ms.slice(5, -2))}]; }))`);
console.log("   colours:", JSON.stringify(col));
for (const k of ["includes", "owns", "shares", "refers"])
    check(col[k].stroke === grey[k] && col[k].end === `url("#arrow-${k}")` && col[k].end_fill === grey[k], `${k}: stroke ${grey[k]}, its arrowhead too`);
check(col.includes.start_fill === grey.includes && col.owns.start_fill === grey.owns && col.refers.start_fill === null, "filled exit markers in the edge's colour");
check(await ev(`getComputedStyle(document.querySelector("#start-shares > path")).stroke`) === grey.shares, "the open circle's outline in shares' grey");
// hot: the markers turn orange with the edge
const hot = await ev(`(() => { const p = document.querySelector("path.edge.member.from-shares"); highlight_ref(p.__data__.row_keys, true); const r = [getComputedStyle(p).markerStart, getComputedStyle(p).markerEnd]; highlight_ref(p.__data__.row_keys, false); return r; })()`);
check(hot[0] === 'url("#start-shares-hot")' && hot[1] === 'url("#arrow-hot")', "hovered: both markers orange: " + JSON.stringify(hot));
// the legend lists the four, under the colours
const le = await ev(`[...document.querySelectorAll("g.legend g.edge-entry")].map(g => [g.querySelector("text").textContent, getComputedStyle(g.querySelector("path.sample")).markerStart, Math.round(g.getBoundingClientRect().top)])`);
console.log("   legend edges:", JSON.stringify(le));
const last_colour = await ev(`Math.round([...document.querySelectorAll("g.legend g.entry")].pop().getBoundingClientRect().top)`);
check(le.length === 4 && le[0][0].startsWith("includes") && le[3][0].startsWith("refers") && le[3][1] === "none" && le[1][1] === 'url("#start-owns")'
      && le.every((x, i) => x[2] > last_colour && (i === 0 || x[2] > le[i - 1][2])), "legend: includes, owns, shares, refers -- below the colours");
// stacking: the strongest kind on top, so a shared entry port shows it
const rank = {refers: 1, shares: 2, owns: 3, includes: 4};
const order = async () => ev(`[...document.querySelectorAll("g.edges > g.edge-g")].map(g => g.__data__.ref_kind || "-")`);
let o = await order();
const nondecreasing = o => o.every((k, i) => i === 0 || (rank[k] || 0) >= (rank[o[i - 1]] || 0));
check(nondecreasing(o), "edge groups stacked by kind, strongest last: " + JSON.stringify(o));
const into_u = await ev(`[...document.querySelectorAll("g.edges > g.edge-g")].filter(g => g.__data__.target === "server/url_router_").map(g => [g.__data__.from, g.__data__.ref_kind])`);
check(into_u.length === 3 && into_u[into_u.length - 1][1] === "includes" && into_u.slice(0, -1).every(x => x[1] === "refers"),
      "into the UrlRouter: the routers' refers under the server's includes: " + JSON.stringify(into_u));
// hovering a refers edge raises it; leaving puts it back
await ev(`(() => { const p = [...document.querySelectorAll("path.edge.member.from-refers")][0]; highlight_ref(p.__data__.row_keys, true); window.__p = p; })()`);
check(await ev(`document.querySelector("g.edges").lastElementChild === window.__p.parentNode`), "hovered: on top");
await ev(`highlight_ref(window.__p.__data__.row_keys, false)`);
check(JSON.stringify(await order()) === JSON.stringify(o), "left: back in rank order");
// tooltip names the kind
check(await ev(`document.querySelector("path.edge.member.from-owns > title").textContent.endsWith("(owns)")`), "tooltip ends with the kind");
await ev(`d3.select("#graph").call(zoom.transform, d3.zoomIdentity.translate(-500, -450).scale(1.6)); 1`); await sleep(300);
fs.writeFileSync(out, Buffer.from((await call("Page.captureScreenshot", {format: "png"})).result.data, "base64"));
console.log(ok ? "ALL OK" : "SOME FAILED");
ws.close(); cl.close(); process.exit(ok ? 0 : 1);
