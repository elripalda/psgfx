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
.hidden{display:none!important}
@media (max-width:900px){.layout{grid-template-columns:1fr}.rail{max-height:none;position:static;border-bottom:1px solid var(--line)}main{border-left:0}.slots{grid-template-columns:1fr}}
@media (prefers-reduced-motion:reduce){*{transition:none!important}}
</style>
</head>
<body>
<header class="top">
  <div class="brand"><img src="/wordmark.png" alt="PSGFX"><span>1.0</span></div>
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
  const any = KINDS.some(k => st.staged[k]);
  $('#apply').disabled = !any; $('#badge').style.display = any ? 'block' : 'none';
}

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
  const a = st.sel; $('#apply').disabled = true; showLog(['Converting and uploading to '+(a.title_name||a.title_id)+'…'], '');
  try {
    const j = await api('/api/apply', {method:'POST', body: JSON.stringify({title_id:a.title_id, src:a.src, kind:a.kind})});
    showLog(j.log, 'ok'); st.staged = {}; refreshPreview(true);
    refreshRailIcon(a);
  } catch (err) { showLog(['Apply failed: '+err.message], 'bad'); $('#apply').disabled = false; }
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

api('/api/config').then(j => { if (j.ip) { $('#ip').value = j.ip; $('#cf').requestSubmit(); } });
</script>
</body>
</html>
)HTML";
