const char *UI_HTML = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>PSGFX</title>
<link rel="icon" href="/mark.svg" type="image/svg+xml">
<style>
@font-face{font-family:Sora;font-weight:300;src:url(/f/sora300.woff2) format("woff2")}
@font-face{font-family:Sora;font-weight:400;src:url(/f/sora400.woff2) format("woff2")}
@font-face{font-family:Sora;font-weight:600;src:url(/f/sora600.woff2) format("woff2")}
@font-face{font-family:Sora;font-weight:700;src:url(/f/sora700.woff2) format("woff2")}
:root{
  --bg:#060913; --bg2:#0c1426; --glass:rgba(160,190,255,.06); --glass-hi:rgba(160,190,255,.11);
  --line:rgba(170,195,255,.13); --text:#eaf0fa; --muted:#8c98ae; --blue:#3d7bff; --blue-hi:#5b90ff;
  --ok:#3ddc97; --warn:#ffb547; --bad:#ff6b6b;
  --r-lg:18px; --r-md:12px; --r-sm:8px;
  font-family:"Sora",system-ui,-apple-system,"Segoe UI",sans-serif;
}
*{box-sizing:border-box}
html,body{margin:0;min-height:100%}
body{background:radial-gradient(1200px 700px at 70% -10%,#13244a 0%,transparent 60%),var(--bg);background-attachment:fixed;color:var(--text);font-size:14px;line-height:1.5}
button,input{font:inherit;color:inherit}
:focus-visible{outline:2px solid var(--blue-hi);outline-offset:2px}

.top{display:flex;align-items:center;gap:20px;padding:16px 24px;border-bottom:1px solid var(--line)}
.brand{display:flex;align-items:flex-end;gap:10px}
.brand img{height:30px;width:auto;display:block}
.brand span{font-weight:300;font-size:13px;color:var(--muted);letter-spacing:0}
.connect{margin-left:auto;display:flex;align-items:center;gap:10px}
.connect label{color:var(--muted);font-size:13px}
.connect input{width:150px;padding:8px 12px;border-radius:var(--r-sm);border:1px solid var(--line);background:var(--glass)}
.dot{width:9px;height:9px;border-radius:50%;background:#4a5570}
.dot.ok{background:var(--ok);box-shadow:0 0 10px var(--ok)}
.dot.busy{background:var(--warn)}
.btn{padding:9px 16px;border-radius:var(--r-sm);border:1px solid var(--line);background:var(--glass-hi);cursor:pointer}
.btn:hover{background:rgba(160,190,255,.18)}
.btn.primary{background:var(--blue);border-color:transparent;font-weight:600}
.btn.primary:hover{background:var(--blue-hi)}
.btn:disabled{opacity:.45;cursor:not-allowed}

.layout{display:grid;grid-template-columns:280px 1fr;min-height:calc(100vh - 66px)}
.rail{padding:16px 12px;overflow:auto;max-height:calc(100vh - 66px);position:sticky;top:0;align-self:start}
.rail h2{font-size:13px;font-weight:600;color:var(--muted);margin:4px 8px 12px}
.app{display:flex;gap:12px;align-items:center;width:100%;padding:8px;border-radius:var(--r-md);border:1px solid transparent;background:none;text-align:left;cursor:pointer}
.app:hover{background:var(--glass)}
.app[aria-current="true"]{background:var(--glass-hi);border-color:var(--line)}
.app .ic{width:48px;height:48px;border-radius:10px;object-fit:cover;background:var(--bg2);flex:none;border:1px solid var(--line)}
.app .ic.none{visibility:hidden}
.rail h2.sec{margin-top:18px}
.more{margin:14px 8px 0;padding:6px 10px;font-size:12px}
.app .n{font-weight:600;font-size:13px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.app .i{color:var(--muted);font-size:12px}
.app.locked{opacity:.5}
.empty{color:var(--muted);padding:8px;font-size:13px}

main{border-left:1px solid var(--line);padding:24px;display:flex;flex-direction:column;gap:20px;max-width:1180px}

/* The PS5 home-screen replica: the one bold element. */
.stage{position:relative;aspect-ratio:16/9;width:min(100%,calc((100vh - 330px)*16/9));min-width:min(100%,520px);margin:0 auto;border-radius:var(--r-lg);overflow:hidden;background:linear-gradient(135deg,#101b33,#060913);border:1px solid var(--line);box-shadow:0 30px 80px rgba(0,0,0,.5)}
.stage .bgimg{position:absolute;inset:0;width:100%;height:100%;object-fit:cover}
.stage .shade{position:absolute;inset:0;background:linear-gradient(180deg,rgba(0,0,0,.35) 0%,transparent 30%,transparent 55%,rgba(0,0,0,.55) 100%)}
.tiles{position:absolute;left:4.5%;top:9%;display:flex;gap:1.1%;align-items:flex-start;width:90%}
.tile{width:6.2%;aspect-ratio:1;border-radius:8%;background:rgba(255,255,255,.12);flex:none}
.tile.sel{width:12.5%;border-radius:7%;overflow:hidden;box-shadow:0 0 0 2px rgba(255,255,255,.9),0 8px 30px rgba(0,0,0,.5);position:relative;background:#1a2440}
.tile.sel img{width:100%;height:100%;object-fit:cover;display:block}
.titlezone{position:absolute;left:4.5%;top:36%;width:42%;height:30%;display:flex;align-items:flex-end}
.titlezone img{max-width:100%;max-height:100%;object-fit:contain;object-position:left bottom}
.titlezone .tname{font-size:clamp(14px,2.2vw,30px);font-weight:600;text-shadow:0 2px 12px rgba(0,0,0,.6)}
.play{position:absolute;left:4.5%;top:70%;padding:.6% 2.2%;border-radius:999px;background:rgba(255,255,255,.92);color:#111;font-weight:600;font-size:clamp(9px,1vw,13px)}
.drop{position:absolute;display:none;align-items:center;justify-content:center;border:2px dashed rgba(255,255,255,.75);background:rgba(61,123,255,.28);border-radius:inherit;font-weight:600;text-shadow:0 1px 6px #000;pointer-events:none}
.stage.dragging .drop{display:flex}
.drop.hot{background:rgba(61,123,255,.55)}
.d-bg{inset:0;border-radius:var(--r-lg)}
.d-icon{left:calc(4.5% - 1px);top:calc(9% - 1px);width:12.5%;aspect-ratio:1;border-radius:10px;font-size:12px;text-align:center}
.stage-hint{position:absolute;right:2%;bottom:3%;font-size:12px;color:rgba(255,255,255,.75)}
.staged-badge{position:absolute;top:3%;right:2%;padding:4px 10px;border-radius:999px;background:rgba(61,123,255,.85);font-size:12px;font-weight:600;display:none}

.namerow{display:grid;gap:6px;max-width:480px}
.namerow label{font-size:12.5px;color:var(--muted)}
.slots{display:grid;grid-template-columns:repeat(2,1fr);gap:14px}
.slot{background:var(--glass);border:1px solid var(--line);border-radius:var(--r-md);padding:14px;display:flex;gap:12px;align-items:center;min-height:92px}
.slot .thumb{width:64px;height:64px;border-radius:10px;background:var(--bg2) center/cover no-repeat;flex:none;border:1px solid var(--line)}
.slot.bgslot .thumb{width:96px;height:54px}
.slot .meta{min-width:0;flex:1}
.slot .t{font-weight:600}
.slot .s{color:var(--muted);font-size:12px}
.slot .s.warn{color:var(--warn)}
.slot .row{display:flex;gap:8px;margin-top:8px}
.slot .row .btn{padding:5px 10px;font-size:12px}

.actions{display:flex;gap:12px;align-items:center;flex-wrap:wrap}
.note{color:var(--muted);font-size:13px}
.log{margin:16px 0 0;background:var(--glass);border:1px solid var(--line);border-radius:var(--r-md);padding:12px 14px 12px 32px;font-size:13px;display:none}
.log li{margin:2px 0}
.log.ok{border-color:rgba(61,220,151,.4)} .log.bad{border-color:rgba(255,107,107,.5);color:#ffd0d0}
.welcome{max-width:560px;color:var(--muted)}
.welcome h1{color:var(--text);font-size:26px;letter-spacing:-.02em;margin:0 0 8px}
/* ── header views ── */
.views{display:flex;gap:4px;margin-left:8px;padding:3px;border-radius:999px;background:var(--glass);border:1px solid var(--line)}
.views button{border:0;background:none;padding:6px 14px;border-radius:999px;color:var(--muted);cursor:pointer;font-weight:600;font-size:13px}
.views button[aria-pressed="true"]{background:var(--glass-hi);color:var(--text)}

/* ── home layout ── */
.lv{display:grid;grid-template-columns:1fr 320px;min-height:calc(100vh - 66px)}
.lv-main{padding:24px;display:flex;flex-direction:column;gap:16px;min-width:0}
.lv-empty{max-width:560px;color:var(--muted)}
.lv-empty h1{color:var(--text);font-size:26px;letter-spacing:-.02em;margin:0 0 8px}
.lstage{position:relative;aspect-ratio:16/9;width:min(100%,calc((100vh - 470px)*16/9));min-width:min(100%,520px);margin:0 auto;border-radius:var(--r-lg);overflow:hidden;background:linear-gradient(135deg,#101b33,#060913);border:1px solid var(--line);box-shadow:0 30px 80px rgba(0,0,0,.5)}
.lstage .bgimg{position:absolute;inset:0;width:100%;height:100%;object-fit:cover}
.ltabs{position:absolute;left:4.5%;top:4%;display:flex;gap:clamp(14px,2.4vw,30px);font-size:clamp(11px,1.4vw,17px);color:rgba(255,255,255,.65)}
.ltabs span{padding-bottom:3px;border-bottom:2px solid transparent;cursor:pointer}
.ltabs span.on{color:#fff;border-color:#fff}
.lrow{position:absolute;left:4.5%;right:0;top:14%;display:flex;gap:.9%;align-items:flex-start;overflow:hidden}
.ltile{flex:none;width:6.2%;aspect-ratio:1;border-radius:8%;background:rgba(255,255,255,.12) center/cover;cursor:pointer;position:relative}
.ltile.sel{width:12.5%;border-radius:7%;box-shadow:0 0 0 2px rgba(255,255,255,.9),0 8px 30px rgba(0,0,0,.5)}
.ltile i,.lcard .im i{position:absolute;inset:0;display:grid;place-items:center;font-style:normal;font-weight:600;font-size:clamp(8px,1vw,14px);color:rgba(255,255,255,.8)}
.ltitle{position:absolute;left:4.5%;top:42%;font-size:clamp(14px,2.2vw,30px);font-weight:600;text-shadow:0 2px 12px rgba(0,0,0,.6);max-width:60%}
.lsub{position:absolute;left:4.5%;top:calc(42% + clamp(20px,3vw,40px));font-size:clamp(10px,1vw,13px);color:rgba(255,255,255,.7)}
.lbar{display:flex;align-items:center;gap:4px;flex-wrap:wrap;width:min(100%,calc((100vh - 470px)*16/9));min-width:min(100%,520px);margin:0 auto}
.ltab{border:0;background:none;padding:7px 12px;border-radius:var(--r-sm);color:var(--muted);cursor:pointer;font-weight:600}
.ltab[aria-pressed="true"]{background:var(--glass-hi);color:var(--text)}
.ltab small{font-weight:400;color:var(--muted);margin-left:4px}
.lsorts{margin-left:auto;display:flex;align-items:center;gap:2px}
.lsorts .btn{padding:5px 10px;font-size:12px}
.lsorts label{display:flex;align-items:center;gap:6px;font-size:12.5px;color:var(--muted);margin-left:8px;cursor:pointer}
.lstrip{display:flex;gap:16px;overflow-x:auto;padding:4px 2px 16px;width:min(100%,calc((100vh - 470px)*16/9));min-width:min(100%,520px);margin:0 auto}
.lgrp{display:flex;flex-direction:column;gap:8px;flex:none}
.lgrp + .lgrp{padding-left:16px;border-left:1px solid var(--line)}
.lgrp>span{font-size:12px;color:var(--muted)}
.lcards{display:flex;gap:10px}
.lcard{flex:none;width:112px;padding:7px;border-radius:var(--r-md);border:1px solid transparent;background:var(--glass);cursor:pointer;position:relative;text-align:left}
.lcard:hover{background:var(--glass-hi)}
.lcard[aria-current="true"]{border-color:var(--blue-hi);background:var(--glass-hi)}
.lcard.drag{opacity:.35}
.lcard.over{box-shadow:-5px 0 0 -2px var(--blue-hi)}
.lcard .im{aspect-ratio:1;border-radius:9px;background:var(--bg2) center/cover;position:relative}
.lcard .nm{margin-top:6px;font-size:12px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.lcard .bd{position:absolute;top:12px;right:12px;font-size:10.5px;font-weight:600;padding:2px 6px;border-radius:999px;background:var(--warn);color:#2a1800}
.lempty{color:var(--muted);padding:20px;border:1px dashed var(--line);border-radius:var(--r-md);flex:1;text-align:center}
.lwarn{width:min(100%,calc((100vh - 470px)*16/9));margin:0 auto;padding:10px 14px;border-radius:var(--r-md);background:rgba(255,181,71,.08);border:1px solid rgba(255,181,71,.3);color:#ffd9a0;font-size:13px}
.lside{border-left:1px solid var(--line);padding:20px;display:flex;flex-direction:column;gap:18px}
.lside h2{font-size:18px;margin:0;letter-spacing:-.01em}
.lside .id{font-size:12px;color:var(--muted)}
.lsec{display:grid;gap:8px}
.lsec>label,.lsec>.lbl{font-size:12.5px;color:var(--muted)}
.lseg{display:grid;grid-template-columns:repeat(3,1fr);padding:3px;border-radius:var(--r-sm);background:var(--glass);border:1px solid var(--line)}
.lseg button{border:0;background:none;padding:7px 0;border-radius:6px;color:var(--muted);cursor:pointer}
.lseg button[aria-pressed="true"]{background:var(--glass-hi);color:var(--text)}
.ltxt{width:100%;padding:9px 11px;border-radius:var(--r-sm);border:1px solid var(--line);background:var(--glass)}
.lnote{font-size:12px;color:var(--muted)}
.linker{border:0;background:none;color:var(--blue-hi);padding:0;cursor:pointer;justify-self:start;font-size:13px}
.lfoot{position:sticky;bottom:0;display:flex;align-items:center;gap:10px;padding:12px 20px;border-top:1px solid var(--line);background:rgba(6,9,19,.92);backdrop-filter:blur(8px);grid-column:1/-1}
.lfoot .pend{color:var(--muted);display:flex;align-items:center;gap:8px}
.lfoot .pend b{width:8px;height:8px;border-radius:50%;background:#4a5570}
.lfoot .pend.has{color:var(--text)} .lfoot .pend.has b{background:var(--warn)}
.lfoot .sp{flex:1}
.scrim{position:fixed;inset:0;display:none;place-items:center;background:rgba(3,5,10,.7);z-index:20}
.scrim.open{display:grid}
.dlg{width:min(560px,92vw);max-height:84vh;overflow:auto;background:#0c1426;border:1px solid var(--line);border-radius:var(--r-lg);padding:22px}
.dlg h3{margin:0 0 6px;font-size:18px}
.dlg p{margin:0 0 12px;color:var(--muted)}
.dlg .li{display:flex;align-items:center;gap:12px;padding:10px 12px;border-radius:var(--r-sm);background:var(--glass);border:1px solid var(--line);margin-bottom:6px}
.dlg .li div{flex:1;min-width:0}
.dlg .li small{display:block;color:var(--muted)}
.dlg .acts{display:flex;justify-content:flex-end;gap:8px;margin-top:16px}
.dlg pre{white-space:pre-wrap;font-size:12px;background:var(--glass);border:1px solid var(--line);border-radius:var(--r-sm);padding:10px;color:var(--muted);max-height:240px;overflow:auto}
.dlg .chk{display:flex;align-items:center;gap:8px;margin:14px 0 8px;cursor:pointer}
.busyv{position:fixed;inset:0;display:none;place-items:center;background:rgba(3,5,10,.55);z-index:30;color:var(--muted)}
.busyv.open{display:grid}
@media (max-width:900px){.lv{grid-template-columns:1fr}.lside{border-left:0;border-top:1px solid var(--line)}}
.hidden{display:none!important}
@media (max-width:900px){.layout{grid-template-columns:1fr}.rail{max-height:none;position:static;border-bottom:1px solid var(--line)}main{border-left:0}.slots{grid-template-columns:1fr}}
@media (prefers-reduced-motion:reduce){*{transition:none!important}}
</style>
</head>
<body>
<header class="top">
  <div class="brand"><img src="/wordmark.png" alt="PSGFX"><span>1.1</span></div>
  <nav class="views" aria-label="View"><button data-v="art" aria-pressed="true">Art</button><button data-v="layout" aria-pressed="false">Home layout</button></nav>
  <form class="connect" id="cf">
    <span class="dot" id="dot" aria-hidden="true"></span>
    <label for="ip">PS5 IP</label>
    <input id="ip" placeholder="192.168.1.50" autocomplete="off" inputmode="decimal">
    <button class="btn" id="cbtn">Connect</button>
  </form>
</header>

<div class="layout">
  <nav class="rail" aria-label="Apps">
    <div id="apps"><p class="empty">Connect to your PS5 to see its apps.</p></div>
  </nav>

  <main>
    <section class="welcome" id="welcome">
      <h1>New art for your homebrew and games.</h1>
      <p>Make sure PS5 Upload's payload is running on your console, then enter your PS5's IP above. Pick an app or game, drop images onto the preview, and apply.</p>
    </section>

    <section id="editor" class="hidden" aria-label="Artwork editor">
      <div class="stage" id="stage">
        <img class="bgimg hidden" id="bgimg" alt="">
        <div class="shade"></div>
        <div class="tiles" aria-hidden="true">
          <div class="tile sel"><img id="tileimg" alt=""></div>
          <div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div>
        </div>
        <div class="titlezone"><div class="tname" id="tname"></div></div>
        <div class="play" aria-hidden="true">Play</div>
        <div class="staged-badge" id="badge">Preview - not applied yet</div>
        <div class="stage-hint">Drop an image on the tile or the background</div>
        <div class="drop d-bg" data-kind="background">Background</div>
        <div class="drop d-icon" data-kind="icon">Icon</div>
      </div>

      <div class="namerow" style="margin-top:20px"><label for="aname">Name on the home screen</label><input class="ltxt" id="aname" maxlength="80" autocomplete="off"></div>

      <div class="slots" style="margin-top:20px">
        <div class="slot" data-kind="icon"><div class="thumb"></div><div class="meta"><div class="t">Icon</div><div class="s">Square, 512 × 512 or larger</div><div class="row"><button class="btn pick">Choose image</button><button class="btn clear hidden">Remove</button></div></div></div>
        <div class="slot bgslot" data-kind="background"><div class="thumb"></div><div class="meta"><div class="t">Background</div><div class="s">16:9, ideally 3840 × 2160</div><div class="row"><button class="btn pick">Choose image</button><button class="btn clear hidden">Remove</button></div></div></div>
      </div>

      <div class="actions" style="margin-top:20px">
        <button class="btn primary" id="apply" disabled>Apply to PS5</button>
        <button class="btn" id="restore">Restore original art</button>
        <span class="note" id="anote">Close the app on your PS5 before applying.</span>
      </div>
      <ul class="log" id="log"></ul>
    </section>
  </main>
</div>
<section class="lv hidden" id="lv" aria-label="Home layout">
  <div class="lv-main">
    <div class="lv-empty" id="lvEmpty">
      <h1>Arrange your home screen.</h1>
      <p>Hide tiles like the PlayStation Store, move apps between Games and Media, rename anything, and put your games in any order. Connect to your PS5 above to start. PSGFX backs up the home screen before every change.</p>
    </div>
    <div id="lvBody" class="hidden" style="display:contents">
      <div id="lvWarn"></div>
      <div class="lstage" id="lstage">
        <img class="bgimg hidden" id="lbg" alt="">
        <div class="shade" style="position:absolute;inset:0;background:linear-gradient(180deg,rgba(0,0,0,.45) 0%,transparent 30%,transparent 55%,rgba(0,0,0,.7) 100%)"></div>
        <div class="ltabs"><span data-w="games">Games</span><span data-w="media">Media</span></div>
        <div class="lrow" id="lrow"></div>
        <div class="ltitle" id="ltitle"></div>
        <div class="lsub" id="lsub"></div>
      </div>
      <div class="lbar">
        <button class="ltab" data-w="games">Games<small id="lcG"></small></button>
        <button class="ltab" data-w="media">Media<small id="lcM"></small></button>
        <button class="ltab" data-w="hidden">Hidden<small id="lcH"></small></button>
        <div class="lsorts" id="lsorts">
          <button class="btn" data-sort="az" title="Sort A to Z">A–Z</button>
          <button class="btn" data-sort="za" title="Sort Z to A">Z–A</button>
          <button class="btn" data-sort="recent" title="Back to recently played order">Recent</button>
          <label title="Keeps this order even after you play other games"><input type="checkbox" id="llock"> Lock order</label>
        </div>
      </div>
      <div class="lstrip" id="lstrip"></div>
    </div>
  </div>
  <aside class="lside" id="lside"><p class="lnote">Select a tile to change it.</p></aside>
  <footer class="lfoot">
    <div class="pend" id="lpend"><b></b><span>No changes</span></div>
    <span class="sp"></span>
    <button class="btn" id="lPresets">Presets</button>
    <button class="btn" id="lBackups">Backups</button>
    <button class="btn" id="lReload">Reload</button>
    <button class="btn" id="lDiscard" disabled>Discard</button>
    <button class="btn primary" id="lApply" disabled>Apply to PS5</button>
  </footer>
</section>
<div class="scrim" id="dlg"><div class="dlg" id="dlgBody"></div></div>
<div class="busyv" id="busyv"><span id="busyT">Working…</span></div>
<input type="file" id="file" accept="image/png,image/jpeg" hidden>

<script>
const $ = s => document.querySelector(s);
const st = { apps: [], sel: null, staged: {} };
const KINDS = ['icon','background'];

async function api(path, opts={}) {
  const r = await fetch(path, opts);
  const ct = r.headers.get('content-type') || '';
  if (ct.includes('json')) { const j = await r.json(); if (j.ok === false) throw new Error(j.error); return j; }
  return r;
}
function setDot(cls){ $('#dot').className = 'dot ' + (cls||''); }
function showLog(lines, cls){ const el=$('#log'); el.innerHTML=''; lines.forEach(l=>{const li=document.createElement('li');li.textContent=l;el.appendChild(li)}); el.className='log '+cls; el.style.display='block'; }

$('#cf').addEventListener('submit', async e => {
  e.preventDefault();
  const ip = $('#ip').value.trim();
  setDot('busy'); $('#cbtn').disabled = true;
  try {
    const j = await api('/api/connect', {method:'POST', body: JSON.stringify({ip})});
    st.apps = j.apps; setDot('ok'); renderApps();
    L.loaded = false; if (!$('#lv').classList.contains('hidden')) lload();
  } catch (err) {
    setDot(''); $('#apps').innerHTML = ''; const p=document.createElement('p'); p.className='empty'; p.textContent = err.message; $('#apps').appendChild(p);
  } finally { $('#cbtn').disabled = false; }
});

st.showHidden = false;
function renderApps(){
  const box = $('#apps'); box.innerHTML = '';
  if (!st.apps.length) { box.innerHTML = '<p class="empty">No apps found.</p>'; return; }
  const groups = [['homebrew','Homebrew'],['game','Installed games']];
  for (const [k, label] of groups) {
    const list = st.apps.filter(a => a.kind === k && !a.hidden);
    if (!list.length) continue;
    const h = document.createElement('h2'); h.className = box.children.length ? 'sec' : ''; h.textContent = label; box.appendChild(h);
    list.forEach(a => box.appendChild(appButton(a)));
  }
  const hid = st.apps.filter(a => a.hidden);
  if (hid.length) {
    const t = document.createElement('button'); t.className = 'btn more';
    t.textContent = st.showHidden ? 'Hide other items' : `Show ${hid.length} other item${hid.length>1?'s':''}`;
    t.onclick = () => { st.showHidden = !st.showHidden; renderApps(); };
    box.appendChild(t);
    if (st.showHidden) {
      const h = document.createElement('h2'); h.className='sec'; h.textContent = 'System and unknown items'; box.appendChild(h);
      hid.forEach(a => box.appendChild(appButton(a)));
    }
  }
  if (!box.querySelector('.app')) box.insertAdjacentHTML('afterbegin','<p class="empty">No homebrew or games found.</p>');
}
function appButton(a){
  const b = document.createElement('button');
  b.className = 'app' + (a.editable ? '' : ' locked');
  b.innerHTML = '<img class="ic" alt="" loading="lazy"><div style="min-width:0"><div class="n"></div><div class="i"></div></div>';
  const im = b.querySelector('img'); im.onerror = () => im.classList.add('none');
  im.src = '/api/icon/' + encodeURIComponent(a.title_id);
  b.querySelector('.n').textContent = a.title_name || a.title_id;
  b.querySelector('.i').textContent = a.editable ? a.title_id : a.title_id + ' - can\'t be edited';
  b.disabled = !a.editable;
  if (st.sel && st.sel.title_id === a.title_id) b.setAttribute('aria-current','true');
  b.onclick = () => select(a, b);
  return b;
}

async function select(a, btn){
  document.querySelectorAll('.app').forEach(x => x.removeAttribute('aria-current'));
  btn.setAttribute('aria-current','true');
  st.sel = a;
  for (const k of KINDS) if (st.staged[k]) { await api('/api/unstage/'+k); }
  st.staged = {};
  $('#welcome').classList.add('hidden'); $('#editor').classList.remove('hidden'); $('#log').style.display='none';
  $('#tname').textContent = a.title_name || a.title_id;
  $('#aname').value = a.title_name || a.title_id;
  const game = a.kind === 'game';
  $('#anote').textContent = game ? 'Close the game first. The PS5 may put back its original art when the game updates.' : 'Close the app on your PS5 before applying.';
  refreshPreview(true);
}

function refreshPreview(loadCurrent){
  const a = st.sel, t = Date.now();
  $('#tileimg').src = st.staged.icon ? '/api/staged/icon?t='+t : '/api/icon/'+encodeURIComponent(a.title_id)+'?t='+t;
  const bg = $('#bgimg');
  if (st.staged.background) { bg.src = '/api/staged/background?t='+t; bg.classList.remove('hidden'); }
  else if (loadCurrent) { bg.classList.add('hidden'); bg.onload=()=>bg.classList.remove('hidden'); bg.onerror=()=>bg.classList.add('hidden');
    bg.src = '/api/current/'+encodeURIComponent(a.title_id)+'?src='+encodeURIComponent(a.src)+'&t='+t; }
  document.querySelectorAll('.slot').forEach(sl => {
    const k = sl.dataset.kind, s = st.staged[k];
    sl.querySelector('.thumb').style.backgroundImage = s ? `url(/api/staged/${k}?t=${t})` : '';
    sl.querySelector('.clear').classList.toggle('hidden', !s);
    const sEl = sl.querySelector('.s');
    if (s && s.note) { sEl.textContent = s.note; sEl.classList.add('warn'); }
    else if (s) { sEl.textContent = 'Ready to apply'; sEl.classList.remove('warn'); }
    else { sEl.textContent = {icon:'Square, 512 × 512 or larger',background:'16:9, ideally 3840 × 2160'}[k]; sEl.classList.remove('warn'); }
  });
  updateApply();
}
const newName = () => { const v = $('#aname').value.trim(); return v && v !== (st.sel.title_name || st.sel.title_id) ? v : null; };
function updateApply(){
  const any = KINDS.some(k => st.staged[k]) || !!newName();
  $('#apply').disabled = !any; $('#badge').style.display = any ? 'block' : 'none';
}
$('#aname').addEventListener('input', () => { $('#tname').textContent = $('#aname').value.trim() || st.sel.title_name || st.sel.title_id; updateApply(); });
$('#aname').addEventListener('keydown', e => { if (e.key === 'Enter' && !$('#apply').disabled) $('#apply').click(); });

async function stage(kind, file){
  const sl = document.querySelector(`.slot[data-kind="${kind}"] .s`);
  sl.textContent = 'Loading…'; sl.classList.remove('warn');
  try {
    const j = await api('/api/stage/'+kind, {method:'POST', body: await file.arrayBuffer()});
    st.staged[kind] = {note: j.note || ''};
  } catch (err) { sl.textContent = err.message; sl.classList.add('warn'); return; }
  refreshPreview(false);
}

let pickKind = null;
document.querySelectorAll('.slot').forEach(sl => {
  sl.querySelector('.pick').onclick = () => { pickKind = sl.dataset.kind; $('#file').value=''; $('#file').click(); };
  sl.querySelector('.clear').onclick = async () => { await api('/api/unstage/'+sl.dataset.kind); delete st.staged[sl.dataset.kind]; refreshPreview(true); };
});
$('#file').onchange = e => { const f = e.target.files[0]; if (f && pickKind) stage(pickKind, f); };

// Drag onto the replica: which zone is under the pointer decides the slot.
const stageEl = $('#stage');
function zoneAt(x, y){
  for (const k of ['icon']) { const r = stageEl.querySelector('.d-'+k).getBoundingClientRect(); if (x>=r.left&&x<=r.right&&y>=r.top&&y<=r.bottom) return k; }
  return 'background';
}
let dragDepth = 0;
stageEl.addEventListener('dragenter', e => { e.preventDefault(); dragDepth++; stageEl.classList.add('dragging'); });
stageEl.addEventListener('dragleave', () => { if (--dragDepth <= 0) { dragDepth=0; stageEl.classList.remove('dragging'); } });
stageEl.addEventListener('dragover', e => {
  e.preventDefault(); const k = zoneAt(e.clientX, e.clientY);
  stageEl.querySelectorAll('.drop').forEach(d => d.classList.toggle('hot', d.dataset.kind === k));
});
stageEl.addEventListener('drop', e => {
  e.preventDefault(); dragDepth=0; stageEl.classList.remove('dragging');
  const f = e.dataTransfer.files[0]; if (f) stage(zoneAt(e.clientX, e.clientY), f);
});
window.addEventListener('dragover', e => e.preventDefault()); window.addEventListener('drop', e => e.preventDefault());

$('#apply').onclick = async () => {
  const a = st.sel, art = KINDS.some(k => st.staged[k]), name = newName(), lines = [];
  $('#apply').disabled = true; showLog([art ? 'Converting and uploading to '+(a.title_name||a.title_id)+'…' : 'Renaming '+(a.title_name||a.title_id)+'…'], '');
  const ids = {title_id:a.title_id, src:a.src, kind:a.kind};
  let artDone = false;
  try {
    if (art) {
      const j = await api('/api/apply', {method:'POST', body: JSON.stringify(ids)});
      lines.push(...j.log); st.staged = {}; artDone = true; refreshRailIcon(a);
    }
    if (name) {
      const j = await api('/api/rename', {method:'POST', body: JSON.stringify({...ids, name})});
      lines.push(...j.log); a.title_name = j.name; L.loaded = false;
      const n = document.querySelector('.app[aria-current="true"] .n'); if (n) n.textContent = j.name;
      $('#aname').value = j.name; $('#tname').textContent = j.name;
    }
    showLog(lines, 'ok');
  } catch (err) { showLog([...lines, (art && !artDone ? 'Apply failed: ' : 'Rename failed: ')+err.message], 'bad'); }
  if (artDone) refreshPreview(true); else updateApply();
};
$('#restore').onclick = async () => {
  const a = st.sel; showLog(['Restoring original art…'], '');
  try {
    const j = await api('/api/restore', {method:'POST', body: JSON.stringify({title_id:a.title_id, src:a.src, kind:a.kind})});
    showLog(j.log, 'ok'); refreshPreview(true);
    refreshRailIcon(a);
  } catch (err) { showLog([err.message], 'bad'); }
};
function refreshRailIcon(a){ const im = document.querySelector('.app[aria-current="true"] .ic'); if (im) { im.classList.remove('none'); im.src = '/api/icon/'+encodeURIComponent(a.title_id)+'?t='+Date.now(); } }

// ─────────────────────────── Home layout ───────────────────────────
const L = { info:null, edits:{}, order:[], base:[], dragged:false, w:'games', sel:null, focus:0, loaded:false, lastPreset:'' };
const LIB = 'NPXS40071';
const esc = s => String(s ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const limg = p => p ? '/api/layout/img?p=' + encodeURIComponent(p) : '';
const ltiles = () => L.info?.tiles ?? [];
const lbyId = id => ltiles().find(t => t.id === id);
function leff(t){
  const e = L.edits[t.id] || {};
  return {...t, where: e.where ?? t.where, name: e.name ?? t.name, changed: !!L.edits[t.id]};
}
function lchanges(){ return Object.keys(L.edits).length + (L.dragged ? 1 : 0); }
function lgroups(w){
  const all = ltiles().map(leff);
  if (w === 'hidden') return [{key:'hidden', label:'Hidden from the home screen', list: all.filter(t => t.where === 'hidden')}];
  const sys = all.filter(t => t.kind === 'system' && t.where === w).sort((a,b) => (b.home - a.home) || (a.homeIdx - b.homeIdx) || (b.access - a.access));
  return [{key:'system', label:'System', list: sys},
          {key:'mine', label: w === 'games' ? 'Games' : 'Apps', list: L.order.map(id => leff(lbyId(id))).filter(t => t.where === w)}];
}
function lrowFor(w){
  const [sys, mine] = lgroups(w).map(g => g.list);
  if (w === 'media') return [...mine, ...sys];
  const lib = sys.filter(t => t.id === LIB), lead = sys.filter(t => t.home && t.id !== LIB), rest = sys.filter(t => !t.home && t.id !== LIB);
  return [...lead, ...mine, ...lib, ...rest];
}
const initials = n => esc(String(n).replace(/[^\p{L}\p{N}\s]/gu, '').split(/\s+/).filter(Boolean).slice(0,2).map(x => x[0]).join('').toUpperCase());
function setImg(el, src, name){
  el.innerHTML = `<i>${initials(name)}</i>`;
  if (!src) return;
  const im = new Image(); im.onload = () => { el.style.backgroundImage = `url('${src}')`; el.innerHTML = ''; }; im.src = src;
}

function lrender(){
  const has = !!L.info;
  $('#lvEmpty').classList.toggle('hidden', has); $('#lvBody').classList.toggle('hidden', !has);
  lfooter();
  if (!has) { $('#lside').innerHTML = '<p class="lnote">Select a tile to change it.</p>'; return; }
  $('#lvWarn').innerHTML = (L.info.warnings || []).map(w => `<div class="lwarn">${esc(w)}</div>`).join('');
  document.querySelectorAll('.ltab').forEach(b => b.setAttribute('aria-pressed', b.dataset.w === L.w));
  $('#lcG').textContent = lgroups('games').reduce((n,g) => n + g.list.length, 0);
  $('#lcM').textContent = lgroups('media').reduce((n,g) => n + g.list.length, 0);
  $('#lcH').textContent = lgroups('hidden')[0].list.length;
  $('#lsorts').classList.toggle('hidden', !L.info.canOrder || L.w === 'hidden');
  // strip
  const gs = lgroups(L.w);
  const strip = $('#lstrip');
  if (!gs.some(g => g.list.length)) strip.innerHTML = `<div class="lempty">${L.w === 'hidden' ? 'Nothing is hidden. Select a tile and choose Hidden to take it off the home screen.' : 'No tiles here.'}</div>`;
  else {
    strip.innerHTML = gs.filter(g => g.list.length).map(g => `<div class="lgrp"><span>${g.label}</span><div class="lcards">${g.list.map(t =>
      `<button class="lcard" data-id="${t.id}" data-g="${g.key}" draggable="${g.key === 'mine' && L.info.canOrder}" ${t.id === L.sel ? 'aria-current="true"' : ''}>
        <div class="im" data-src="${esc(limg(t.icon))}" data-n="${esc(t.name)}"></div>${t.changed ? '<span class="bd">Edited</span>' : ''}
        <div class="nm" title="${esc(t.name)}">${esc(t.name)}</div></button>`).join('')}</div></div>`).join('');
    strip.querySelectorAll('.im').forEach(el => setImg(el, el.dataset.src, el.dataset.n));
    strip.querySelector('[aria-current="true"]')?.scrollIntoView({block:'nearest', inline:'nearest'});
  }
  // preview
  const pw = L.w === 'hidden' ? 'games' : L.w;
  document.querySelectorAll('.ltabs span').forEach(s => s.classList.toggle('on', s.dataset.w === pw));
  const row = lrowFor(pw);
  if (L.sel) { const i = row.findIndex(t => t.id === L.sel); if (i >= 0) L.focus = i; }
  L.focus = Math.max(0, Math.min(L.focus, row.length - 1));
  $('#lrow').innerHTML = row.slice(0, 16).map((t,i) => `<div class="ltile ${i === L.focus ? 'sel' : ''}" data-id="${t.id}" data-src="${esc(limg(t.icon))}" data-n="${esc(t.name)}"></div>`).join('');
  $('#lrow').querySelectorAll('.ltile').forEach(el => setImg(el, el.dataset.src, el.dataset.n));
  const f = row[L.focus];
  $('#ltitle').textContent = f ? f.name : 'Nothing on this tab';
  $('#lsub').textContent = f && f.kind === 'system' ? 'System' : '';
  const bg = $('#lbg');
  if (f && f.bg) { bg.onload = () => bg.classList.remove('hidden'); bg.onerror = () => bg.classList.add('hidden'); bg.src = limg(f.bg); }
  else bg.classList.add('hidden');
  lside();
}

function lside(){
  const box = $('#lside'), t0 = L.sel && lbyId(L.sel);
  if (!t0) { box.innerHTML = '<p class="lnote">Select a tile to change it.</p>'; return; }
  const t = leff(t0), e = L.edits[t.id] || {};
  const art = st.apps.find(a => a.title_id === t.id && a.editable);
  box.innerHTML = `<div><h2>${esc(t.name)}</h2><div class="id">${esc(t.id)}${t.kind === 'system' ? ' · system tile' : ''}</div></div>
    <div class="lsec"><span class="lbl">Shows on</span><div class="lseg">${['games','media','hidden'].map(v =>
      `<button data-where="${v}" aria-pressed="${t.where === v}">${v[0].toUpperCase() + v.slice(1)}</button>`).join('')}</div></div>
    <div class="lsec"><label for="lname">Name on the home screen</label><input class="ltxt" id="lname" value="${esc(t.name)}" maxlength="80">
      ${e.name != null ? '<button class="linker" id="lname0">Use original name</button>' : ''}</div>
    ${art ? '<div class="lsec"><span class="lbl">Icon and background</span><button class="btn" id="lart">Edit art</button></div>' : ''}
    ${t.changed ? '<button class="linker" id="lundo" style="color:var(--bad)">Undo changes to this tile</button>' : ''}`;
}

function lfooter(){
  const n = lchanges(), p = $('#lpend');
  p.classList.toggle('has', n > 0);
  p.querySelector('span').textContent = n ? `${n} unsaved change${n > 1 ? 's' : ''}` : 'No changes';
  $('#lApply').disabled = !n || !L.info; $('#lDiscard').disabled = !n;
  ['#lPresets','#lReload'].forEach(s => $(s).disabled = !L.info);
}

function lsetEdit(id, key, val){
  const t = lbyId(id), e = L.edits[id] ||= {};
  if (val === t[key] || val === '' || val == null) delete e[key]; else e[key] = val;
  if (!Object.keys(e).length) delete L.edits[id];
  lrender();
}
function lreset(){
  L.order = ltiles().filter(t => t.kind !== 'system').sort((a,b) => b.access - a.access).map(t => t.id);
  L.base = [...L.order]; L.edits = {}; L.dragged = false; $('#llock').checked = false;
}

function busyv(on, t){ $('#busyv').classList.toggle('open', on); if (t) $('#busyT').textContent = t; }
function dlg(html){ $('#dlgBody').innerHTML = html; $('#dlg').classList.add('open'); }
function dlgClose(){ $('#dlg').classList.remove('open'); }
$('#dlg').addEventListener('click', e => { if (e.target.id === 'dlg' || e.target.closest('[data-close]')) dlgClose(); });

async function lload(){
  busyv(true, 'Reading your home screen…');
  try { L.info = await api('/api/layout/load', {method:'POST', body: JSON.stringify({})}); lreset(); L.loaded = true; }
  catch (err) { L.info = null; $('#lvEmpty').innerHTML = `<h1>Couldn't read the home screen</h1><p>${esc(err.message)}</p>`; }
  busyv(false); lrender();
}

// views
function setView(v){
  document.querySelectorAll('.views button').forEach(b => b.setAttribute('aria-pressed', b.dataset.v === v));
  document.querySelector('.layout').classList.toggle('hidden', v !== 'art');
  $('#lv').classList.toggle('hidden', v !== 'layout');
  if (v === 'layout' && !L.loaded && $('#dot').classList.contains('ok')) lload();
  lrender();
}
document.querySelectorAll('.views button').forEach(b => b.onclick = () => setView(b.dataset.v));

// interactions
document.querySelectorAll('.ltab').forEach(b => b.onclick = () => { L.w = b.dataset.w; L.focus = 0; lrender(); });
document.querySelectorAll('.ltabs span').forEach(s => s.onclick = () => { L.w = s.dataset.w; L.focus = 0; L.sel = null; lrender(); });
$('#lstrip').addEventListener('click', e => { const c = e.target.closest('.lcard'); if (c) { L.sel = c.dataset.id; lrender(); } });
$('#lrow').addEventListener('click', e => { const c = e.target.closest('.ltile'); if (c) { L.sel = c.dataset.id; lrender(); } });
document.addEventListener('keydown', e => {
  if ($('#lv').classList.contains('hidden') || !L.info || /INPUT|TEXTAREA/.test(document.activeElement.tagName) || $('#dlg').classList.contains('open')) return;
  if (e.key !== 'ArrowLeft' && e.key !== 'ArrowRight') return;
  const row = lrowFor(L.w === 'hidden' ? 'games' : L.w); if (!row.length) return;
  L.focus = Math.max(0, Math.min(row.length - 1, L.focus + (e.key === 'ArrowRight' ? 1 : -1)));
  L.sel = row[L.focus].id; lrender(); e.preventDefault();
});
$('#lside').addEventListener('click', e => {
  const id = L.sel; if (!id) return;
  const w = e.target.closest('[data-where]'); if (w) return lsetEdit(id, 'where', w.dataset.where);
  if (e.target.closest('#lname0')) return lsetEdit(id, 'name', null);
  if (e.target.closest('#lundo')) { delete L.edits[id]; return lrender(); }
  if (e.target.closest('#lart')) {
    setView('art');
    const btn = [...document.querySelectorAll('.app')].find(b => b.querySelector('.i').textContent.startsWith(id));
    const a = st.apps.find(x => x.title_id === id);
    if (a && a.hidden && !st.showHidden) { st.showHidden = true; renderApps(); }
    const b2 = [...document.querySelectorAll('.app')].find(b => b.querySelector('.i').textContent.startsWith(id)) || btn;
    if (a && b2) select(a, b2);
  }
});
$('#lside').addEventListener('change', e => { if (e.target.id === 'lname') lsetEdit(L.sel, 'name', e.target.value.trim()); });
$('#lside').addEventListener('keydown', e => { if (e.target.id === 'lname' && e.key === 'Enter') e.target.blur(); });

let ldrag = null;
$('#lstrip').addEventListener('dragstart', e => { const c = e.target.closest('.lcard'); if (!c || c.dataset.g !== 'mine') return; ldrag = c.dataset.id; c.classList.add('drag'); e.dataTransfer.effectAllowed = 'move'; e.dataTransfer.setData('text/plain', ''); });
$('#lstrip').addEventListener('dragend', () => { ldrag = null; document.querySelectorAll('.lcard').forEach(c => c.classList.remove('drag','over')); });
$('#lstrip').addEventListener('dragover', e => { const c = e.target.closest('.lcard'); if (!ldrag || !c || c.dataset.g !== 'mine') return; e.preventDefault(); document.querySelectorAll('.lcard.over').forEach(x => x.classList.remove('over')); c.classList.add('over'); });
$('#lstrip').addEventListener('drop', e => {
  e.preventDefault(); const c = e.target.closest('.lcard'); if (!ldrag || !c || c.dataset.g !== 'mine' || c.dataset.id === ldrag) return;
  L.order.splice(L.order.indexOf(ldrag), 1); L.order.splice(L.order.indexOf(c.dataset.id), 0, ldrag);
  L.dragged = true; L.sel = ldrag; lrender();
});
document.querySelectorAll('[data-sort]').forEach(b => b.onclick = () => {
  const k = b.dataset.sort;
  if (k === 'recent') { L.order = [...L.base]; L.dragged = false; return lrender(); }
  const key = id => leff(lbyId(id)).name.replace(/^[^\p{L}\p{N}]+/u, '').toLocaleLowerCase();
  L.order.sort((a,b) => key(a).localeCompare(key(b)) * (k === 'za' ? -1 : 1));
  L.dragged = true; lrender();
});
$('#lDiscard').onclick = () => { lreset(); lrender(); };
$('#lReload').onclick = () => { if (lchanges() && !confirm('Reloading discards your unsaved changes. Continue?')) return; lload(); };

const presetData = () => ({edits: L.edits, order: L.order, dragged: L.dragged, lock: $('#llock').checked});
function describe(e){
  const p = [];
  if (e.where) p.push(e.where === 'hidden' ? 'hide' : 'show on ' + e.where);
  if (e.name != null) p.push(`rename to “${e.name}”`);
  return p.join(', ');
}
$('#lApply').onclick = () => {
  const lines = Object.entries(L.edits).map(([id, e]) => `<div class="li"><div>${esc(lbyId(id).name)}<small>${esc(describe(e))}</small></div></div>`);
  if (L.dragged) lines.push(`<div class="li"><div>${$('#llock').checked ? 'Locked order' : 'New order'}<small>${esc(L.order.slice(0,6).map(id => leff(lbyId(id)).name).join(' → '))}${L.order.length > 6 ? ' …' : ''}</small></div></div>`);
  dlg(`<h3>Apply ${lchanges()} change${lchanges() > 1 ? 's' : ''} to your PS5?</h3>
    <p>PSGFX saves a backup of your current home screen first. Restart the PS5 afterwards to see the result.</p>
    ${lines.join('')}
    <label class="chk"><input type="checkbox" id="dSave" checked> Also save this layout as a preset</label>
    <input class="ltxt" id="dName" value="${esc(L.lastPreset || 'My layout')}" maxlength="60" aria-label="Preset name">
    <p class="lnote" style="margin-top:6px">Re-apply it in one click after a firmware update or database rebuild resets your home screen.</p>
    <div class="acts"><button class="btn" data-close>Cancel</button><button class="btn primary" id="dGo">Apply changes</button></div>`);
  $('#dSave').onchange = e => $('#dName').disabled = !e.target.checked;
  $('#dGo').onclick = async () => {
    if ($('#dSave').checked) {
      const name = $('#dName').value.trim() || 'My layout';
      try { await api('/api/layout/preset/save', {method:'POST', body: JSON.stringify({name, data: presetData()})}); L.lastPreset = name; }
      catch (err) { if (!confirm(`Couldn't save the preset (${err.message}). Apply anyway?`)) return; }
    }
    dlgClose(); busyv(true, 'Backing up and applying…');
    const edits = {};
    for (const [id, e] of Object.entries(L.edits)) edits[id] = e;
    try {
      const j = await api('/api/layout/apply', {method:'POST', body: JSON.stringify({edits, recent: L.order, recentChanged: L.dragged, lock: $('#llock').checked})});
      L.info = j.layout; lreset(); lrender();
      dlg(`<h3>Changes applied</h3><p>Restart your PS5 to see the new home screen. If anything looks wrong, restore backup ${esc(j.backup)} from Backups.</p><pre>${esc(j.log.join('\n'))}</pre><div class="acts"><button class="btn primary" data-close>Done</button></div>`);
    } catch (err) {
      dlg(`<h3>Changes weren't applied</h3><p>${esc(err.message)}</p><div class="acts"><button class="btn primary" data-close>OK</button></div>`);
    }
    busyv(false);
  };
};
$('#lBackups').onclick = async () => {
  const j = await api('/api/layout/backups');
  dlg(`<h3>Home screen backups</h3><p>Saved in the "PSGFX Layout" folder next to PSGFX.exe, before every change, plus one of the very first home screen PSGFX read.</p>
    ${j.backups.length ? j.backups.map(b => `<div class="li"><div>${esc(b.label)}<small>${esc(b.created)} · ${esc(b.ps5)}</small></div><button class="btn" data-restore="${esc(b.id)}" ${$('#dot').classList.contains('ok') ? '' : 'disabled'}>Restore</button></div>`).join('') : '<p class="lnote">No backups yet. One is made the first time you open Home layout.</p>'}
    <div class="acts"><button class="btn primary" data-close>Close</button></div>`);
  document.querySelectorAll('[data-restore]').forEach(b => b.onclick = async () => {
    if (!confirm('Upload this backup to your PS5? Your current home screen is replaced by it.')) return;
    dlgClose(); busyv(true, 'Restoring backup…');
    try { const r = await api('/api/layout/restore', {method:'POST', body: JSON.stringify({id: b.dataset.restore})}); L.info = r.layout; lreset(); lrender();
      dlg('<h3>Backup restored</h3><p>Restart your PS5 to load it.</p><div class="acts"><button class="btn primary" data-close>Done</button></div>'); }
    catch (err) { alert(err.message); }
    busyv(false);
  });
};
$('#lPresets').onclick = async () => {
  const j = await api('/api/layout/presets');
  dlg(`<h3>Layout presets</h3><p>Load one, check the preview, then Apply to PS5.</p>
    ${j.presets.length ? j.presets.map(n => `<div class="li"><div>${esc(n)}</div><button class="btn" data-load="${esc(n)}">Load</button></div>`).join('') : '<p class="lnote">No presets yet. Tick "Also save this layout as a preset" when you apply.</p>'}
    <div class="acts"><button class="btn primary" data-close>Close</button></div>`);
  document.querySelectorAll('[data-load]').forEach(b => b.onclick = async () => {
    const p = (await api('/api/layout/preset/load', {method:'POST', body: JSON.stringify({name: b.dataset.load})})).data;
    lreset(); let n = 0;
    for (const [id, e] of Object.entries(p.edits || {})) if (lbyId(id)) { L.edits[id] = {...e}; n++; }
    if (p.dragged && p.order) { const known = p.order.filter(id => L.order.includes(id)); L.order = [...known, ...L.order.filter(id => !known.includes(id))]; L.dragged = true; }
    $('#llock').checked = !!p.lock; L.lastPreset = b.dataset.load;
    dlgClose(); lrender();
    dlg(`<h3>Preset loaded</h3><p>${n} tile${n === 1 ? '' : 's'} matched this console${p.dragged ? ', plus your order' : ''}. Check the preview, then Apply to PS5.</p><div class="acts"><button class="btn primary" data-close>OK</button></div>`);
  });
};
window.addEventListener('beforeunload', e => { if (lchanges()) { e.preventDefault(); e.returnValue = ''; } });

api('/api/config').then(j => { if (j.ip) { $('#ip').value = j.ip; $('#cf').requestSubmit(); } });
</script>
</body>
</html>
)HTML";
