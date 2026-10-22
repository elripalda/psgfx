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
  --bg:#04060f; --ink:#eef3ff; --muted:#97a3bb; --dim:#5d6982;
  --cyan:#3fd0ff; --blue:#2f6bff; --indigo:#2a17c4; --violet:#8a6bff; --pink:#ff74d4;
  --ok:#3ddc97; --warn:#ffb547; --bad:#ff6b7a;
  --line:rgba(255,255,255,.085); --line-hi:rgba(255,255,255,.16);
  --fill:rgba(255,255,255,.045); --fill-hi:rgba(255,255,255,.085);
  --r-xl:26px; --r-lg:20px; --r-md:14px; --r-sm:11px;
  --ease:cubic-bezier(.2,.8,.2,1); --spring:cubic-bezier(.3,1.35,.45,1);
  --top:76px;
  font-family:"Sora",system-ui,-apple-system,"Segoe UI",sans-serif;
}
*{box-sizing:border-box}
html,body{margin:0;min-height:100%}
body{background:var(--bg);color:var(--ink);font-size:15.5px;line-height:1.5;overflow-x:hidden}
/* Slow-moving light behind the glass. */
body::before{content:"";position:fixed;inset:-20%;z-index:-1;pointer-events:none;
  background:radial-gradient(38% 42% at 18% 12%,rgba(47,107,255,.30),transparent 70%),
             radial-gradient(34% 40% at 86% 8%,rgba(138,107,255,.22),transparent 70%),
             radial-gradient(40% 46% at 70% 92%,rgba(63,208,255,.16),transparent 70%),
             radial-gradient(30% 34% at 8% 88%,rgba(255,116,212,.08),transparent 70%);
  filter:blur(20px);animation:drift 38s ease-in-out infinite alternate}
@keyframes drift{0%{transform:translate3d(0,0,0) rotate(0)}50%{transform:translate3d(3%,-2%,0) rotate(4deg)}100%{transform:translate3d(-3%,2%,0) rotate(-3deg)}}
button,input,select{font:inherit;color:inherit}
:focus-visible{outline:2px solid var(--cyan);outline-offset:2px}
::selection{background:rgba(63,208,255,.35)}
::-webkit-scrollbar{width:12px;height:12px}
::-webkit-scrollbar-track,::-webkit-scrollbar-corner{background:transparent}
::-webkit-scrollbar-thumb{background:rgba(160,190,255,.18);border-radius:12px;border:3px solid transparent;background-clip:padding-box}
::-webkit-scrollbar-thumb:hover{background-color:rgba(160,190,255,.3)}
::-webkit-scrollbar-thumb:active{background-color:var(--blue)}
@supports not selector(::-webkit-scrollbar){*{scrollbar-width:thin;scrollbar-color:rgba(160,190,255,.22) transparent}}

/* ── glass ── */
.glass{position:relative;background:linear-gradient(160deg,rgba(255,255,255,.075),rgba(255,255,255,.018) 60%);border:1px solid var(--line);
  backdrop-filter:blur(26px) saturate(160%);-webkit-backdrop-filter:blur(26px) saturate(160%);
  box-shadow:inset 0 1px 0 rgba(255,255,255,.07),0 24px 60px rgba(0,0,0,.35)}
/* The chromatic edge: a thin iridescent rim, brightest at the top-left. */
.chroma::before{content:"";position:absolute;inset:-1px;border-radius:inherit;padding:1px;pointer-events:none;
  background:linear-gradient(135deg,rgba(63,208,255,.55),rgba(138,107,255,.28) 30%,rgba(255,116,212,.10) 55%,rgba(255,255,255,0) 70%,rgba(63,208,255,.25));
  -webkit-mask:linear-gradient(#000 0 0) content-box,linear-gradient(#000 0 0);-webkit-mask-composite:xor;mask-composite:exclude}

/* ── buttons ── */
.btn{position:relative;display:inline-flex;align-items:center;justify-content:center;gap:8px;padding:11px 20px;border-radius:999px;border:1px solid var(--line-hi);
  background:var(--fill-hi);cursor:pointer;font-weight:600;font-size:14.5px;white-space:nowrap;
  transition:background .2s,border-color .2s,transform .15s var(--ease),box-shadow .25s,opacity .2s}
.btn:hover{background:rgba(255,255,255,.13);border-color:rgba(255,255,255,.24)}
.btn:active{transform:scale(.97)}
.btn.primary{border-color:transparent;background:linear-gradient(120deg,var(--cyan),var(--blue) 45%,var(--indigo));background-size:160% 100%;background-position:0 0;
  box-shadow:0 8px 28px rgba(47,107,255,.38),inset 0 1px 0 rgba(255,255,255,.35)}
.btn.primary:hover{background-position:100% 0;box-shadow:0 10px 36px rgba(63,208,255,.45),inset 0 1px 0 rgba(255,255,255,.4)}
.btn.ghost{background:transparent;border-color:transparent;color:var(--muted)}
.btn.ghost:hover{background:var(--fill);color:var(--ink)}
.btn.danger{color:#ffc2c9}
.btn.sm{padding:7px 14px;font-size:13px}
.btn:disabled{opacity:.4;cursor:not-allowed;transform:none;box-shadow:none}
.spin{width:16px;height:16px;border-radius:50%;border:2px solid rgba(255,255,255,.3);border-top-color:#fff;animation:spin .8s linear infinite;flex:none}
@keyframes spin{to{transform:rotate(360deg)}}

/* ── segmented controls with a sliding highlight ── */
.seg{position:relative;display:inline-flex;padding:4px;border-radius:999px;background:rgba(0,0,0,.25);border:1px solid var(--line)}
.seg button{position:relative;z-index:1;border:0;background:none;padding:9px 20px;border-radius:999px;color:var(--muted);cursor:pointer;font-weight:600;font-size:14.5px;transition:color .25s;display:flex;align-items:center;gap:7px}
.seg button:hover{color:var(--ink)}
.seg button[aria-pressed="true"]{color:#fff}
.seg .ind{position:absolute;z-index:0;top:4px;bottom:4px;left:0;width:0;border-radius:999px;
  background:linear-gradient(120deg,rgba(63,208,255,.35),rgba(47,107,255,.45));box-shadow:0 4px 18px rgba(47,107,255,.35),inset 0 1px 0 rgba(255,255,255,.25);
  transition:left .38s var(--spring),width .38s var(--spring)}
.seg small{font-weight:400;color:inherit;opacity:.7}
.pip{width:7px;height:7px;border-radius:50%;background:var(--warn);box-shadow:0 0 8px var(--warn);display:none}
.pip.on{display:block;animation:pop .35s var(--spring)}
@keyframes pop{from{transform:scale(0)}to{transform:scale(1)}}

/* ── header ── */
.top{position:sticky;top:0;z-index:15;height:var(--top);display:flex;align-items:center;gap:24px;padding:0 28px;border-width:0 0 1px;border-radius:0;box-shadow:none}
.brand{display:flex;align-items:center;gap:12px}
.brand img{height:30px;width:auto;display:block}
.ver{font-size:12px;font-weight:600;padding:3px 9px;border-radius:999px;background:linear-gradient(120deg,rgba(63,208,255,.18),rgba(138,107,255,.22));border:1px solid rgba(63,208,255,.3);color:#bfeaff}
.top .views{margin:0 auto}
.connpill{display:flex;align-items:center;gap:10px;padding:9px 16px 9px 14px;border-radius:999px;border:1px solid var(--line-hi);background:var(--fill);cursor:pointer;font-weight:600;font-size:14px;transition:background .2s,border-color .2s}
.connpill:hover{background:var(--fill-hi);border-color:rgba(255,255,255,.25)}
.connpill .sub{color:var(--muted);font-weight:400}
.dot{width:10px;height:10px;border-radius:50%;background:#58627a;flex:none;transition:background .3s,box-shadow .3s}
.dot.ok{background:var(--ok);box-shadow:0 0 12px var(--ok)}
.dot.busy{background:var(--warn);animation:blink 1s ease-in-out infinite}
.dot.bad{background:var(--bad)}
@keyframes blink{50%{opacity:.35}}

/* ── views ── */
.view{display:none}
.view.on{display:grid;animation:viewIn .45s var(--ease)}
@keyframes viewIn{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:none}}
@keyframes fadeUp{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:none}}

/* ── Art view ── */
.art{grid-template-columns:340px 1fr;min-height:calc(100vh - var(--top))}
.rail{position:sticky;top:var(--top);align-self:start;height:calc(100vh - var(--top));display:flex;flex-direction:column;border-width:0 1px 0 0;border-radius:0;box-shadow:none;background:linear-gradient(180deg,rgba(255,255,255,.035),rgba(255,255,255,.01))}
.search{padding:18px 16px 8px}
.search input{width:100%;padding:12px 16px 12px 42px;border-radius:999px;border:1px solid var(--line);background:rgba(0,0,0,.25) url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='18' height='18' fill='none' stroke='%2397a3bb' stroke-width='2' stroke-linecap='round'%3E%3Ccircle cx='8' cy='8' r='6'/%3E%3Cpath d='m13 13 4 4'/%3E%3C/svg%3E") 16px center no-repeat;font-size:14.5px;transition:border-color .2s,background-color .2s}
.search input:focus{outline:0;border-color:rgba(63,208,255,.5);background-color:rgba(0,0,0,.35)}
.apps{overflow:auto;padding:4px 12px 20px;flex:1}
.apps h2{font-size:12.5px;font-weight:600;letter-spacing:.06em;text-transform:uppercase;color:var(--dim);margin:16px 10px 8px}
.app{display:flex;gap:14px;align-items:center;width:100%;padding:10px;border-radius:var(--r-md);border:1px solid transparent;background:none;text-align:left;cursor:pointer;
  transition:background .2s,border-color .2s,transform .2s var(--ease);animation:fadeUp .4s var(--ease) backwards;animation-delay:calc(var(--i,0) * 25ms)}
.app:hover{background:var(--fill);transform:translateX(3px)}
.app[aria-current="true"]{background:linear-gradient(120deg,rgba(63,208,255,.14),rgba(47,107,255,.10));border-color:rgba(63,208,255,.35);box-shadow:0 6px 20px rgba(47,107,255,.15)}
.app .ic{width:56px;height:56px;border-radius:13px;object-fit:cover;background:rgba(255,255,255,.06);flex:none;box-shadow:0 4px 14px rgba(0,0,0,.35)}
.app .ic.none{visibility:hidden}
.app .n{font-weight:600;font-size:15px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.app .i{color:var(--muted);font-size:12.5px}
.app.locked{opacity:.45;cursor:default}
.app.locked:hover{transform:none;background:none}
.more{margin:14px 10px 0}
.empty{color:var(--muted);padding:12px 10px;font-size:14px}

.amain{min-width:0;display:flex;flex-direction:column}
.acontent{flex:1;padding:28px max(28px,calc((100% - 1180px)/2)) 28px;display:flex;flex-direction:column;gap:22px}
.welcome{margin:auto;max-width:640px;text-align:center;padding:40px 0;animation:fadeUp .6s var(--ease)}
.welcome .mk{width:88px;height:88px;filter:drop-shadow(0 12px 30px rgba(47,107,255,.45));animation:float 6s ease-in-out infinite}
@keyframes float{50%{transform:translateY(-8px)}}
.welcome h1{font-size:40px;line-height:1.15;letter-spacing:-.03em;margin:18px 0 12px;
  background:linear-gradient(110deg,#fff 30%,#bfe9ff 55%,#c9b8ff 75%);-webkit-background-clip:text;background-clip:text;color:transparent}
.welcome p{color:var(--muted);font-size:16.5px;margin:0 auto 26px;max-width:520px}
.ahead{display:flex;align-items:flex-end;gap:16px;animation:fadeUp .4s var(--ease)}
.ahead h2{margin:0;font-size:28px;letter-spacing:-.02em;line-height:1.2}
.ahead .sub{color:var(--muted);font-size:13.5px}
.ahead .sp{flex:1}

/* The PS5 home-screen replica. Everything inside is sized in cqw so it scales like a real screen. */
.stage{position:relative;container-type:inline-size;aspect-ratio:16/9;width:min(100%,calc((100vh - 470px)*16/9));min-width:min(100%,520px);margin:0 auto;border-radius:var(--r-xl);overflow:hidden;
  background:linear-gradient(135deg,#101b33,#060913);box-shadow:0 0 0 1px var(--line-hi),0 40px 90px rgba(0,0,0,.55),0 0 80px rgba(47,107,255,.12)}
.stage::after{content:"";position:absolute;inset:0;border-radius:inherit;pointer-events:none;box-shadow:inset 0 1px 0 rgba(255,255,255,.12)}
.bgimg{position:absolute;inset:0;width:100%;height:100%;object-fit:cover;opacity:0;transition:opacity .5s var(--ease)}
.bgimg.on{opacity:1}
.shade{position:absolute;inset:0;background:linear-gradient(180deg,rgba(0,0,0,.42) 0%,transparent 30%,transparent 55%,rgba(0,0,0,.6) 100%)}
.tiles{position:absolute;left:4.5cqw;top:5cqw;display:flex;gap:1.1cqw;align-items:flex-start}
.tile{width:6.2cqw;aspect-ratio:1;border-radius:.9cqw;background:rgba(255,255,255,.13);flex:none}
.tile.sel{width:12.5cqw;border-radius:1.3cqw;overflow:hidden;position:relative;background:#1a2440;outline:.28cqw solid #fff;outline-offset:.32cqw;box-shadow:0 1.2cqw 3cqw rgba(0,0,0,.5)}
.tile.sel img{width:100%;height:100%;object-fit:cover;display:block;transition:opacity .3s}
.titlezone{position:absolute;left:4.5cqw;top:22cqw;width:45cqw;display:flex;align-items:flex-end}
.tname{font-size:3.2cqw;font-weight:600;text-shadow:0 .3cqw 1.5cqw rgba(0,0,0,.6);transition:opacity .2s,transform .2s}
.play{position:absolute;left:4.5cqw;top:38cqw;padding:.8cqw 3cqw;border-radius:999px;background:rgba(255,255,255,.94);color:#111;font-weight:600;font-size:1.35cqw}
.drop{position:absolute;display:none;align-items:center;justify-content:center;border:2px dashed rgba(255,255,255,.75);background:rgba(47,107,255,.28);border-radius:inherit;font-weight:600;font-size:1.6cqw;text-shadow:0 1px 6px #000;pointer-events:none;transition:background .2s}
.stage.dragging .drop{display:flex}
.drop.hot{background:rgba(63,208,255,.5)}
.d-bg{inset:0}
.d-icon{left:4.5cqw;top:5cqw;width:12.5cqw;aspect-ratio:1;border-radius:1.3cqw;text-align:center}
.stage-hint{position:absolute;right:2.2cqw;bottom:2cqw;font-size:1.25cqw;color:rgba(255,255,255,.78)}
.staged-badge{position:absolute;top:2cqw;right:2.2cqw;padding:.6cqw 1.4cqw;border-radius:999px;background:linear-gradient(120deg,rgba(63,208,255,.9),rgba(47,107,255,.9));font-size:1.2cqw;font-weight:600;box-shadow:0 .5cqw 2cqw rgba(47,107,255,.5);
  opacity:0;transform:translateY(-6px) scale(.96);transition:opacity .3s,transform .35s var(--spring);pointer-events:none}
.staged-badge.on{opacity:1;transform:none}

.etabs{align-self:center}
.panel{display:none;flex-direction:column;gap:16px}
.panel.on{display:flex;animation:fadeUp .35s var(--ease)}
.card{border-radius:var(--r-lg);padding:20px}
.slot{display:flex;gap:18px;align-items:center}
.slot .thumb{width:84px;height:84px;border-radius:16px;background:rgba(0,0,0,.3) center/cover no-repeat;flex:none;border:1px solid var(--line);transition:background-image .3s}
.slot.bgslot .thumb{width:150px;height:84px}
.slot.musicslot .thumb{display:grid;place-items:center;font-size:34px;color:var(--cyan);background:radial-gradient(circle at 30% 20%,rgba(63,208,255,.25),rgba(0,0,0,.3))}
.slot .meta{min-width:0;flex:1}
.slot .t,.card .t{font-weight:600;font-size:16.5px}
.slot .s,.card .s{color:var(--muted);font-size:13.5px}
.slot .s.warn{color:var(--warn)}
.slot .row{display:flex;gap:10px;margin-top:12px;flex-wrap:wrap}
.chead{display:flex;align-items:center;gap:14px;flex-wrap:wrap}
.chead>div{flex:1;min-width:200px}
.pgrid{display:flex;gap:12px;flex-wrap:wrap;margin-top:14px}
.pick{position:relative;width:92px;height:92px;padding:0;border-radius:18px;border:2px solid transparent;background:rgba(0,0,0,.3) center/cover no-repeat;cursor:pointer;
  transition:transform .25s var(--spring),border-color .2s,box-shadow .25s;animation:fadeUp .35s var(--ease) backwards;animation-delay:calc(var(--i,0) * 30ms)}
.pick:hover{transform:translateY(-3px) scale(1.04);box-shadow:0 10px 26px rgba(0,0,0,.45)}
.pick[aria-pressed="true"]{border-color:var(--cyan);box-shadow:0 0 0 4px rgba(63,208,255,.2),0 10px 26px rgba(47,107,255,.35)}
.pick .by{position:absolute;left:0;right:0;bottom:0;padding:14px 6px 5px;border-radius:0 0 16px 16px;background:linear-gradient(transparent,rgba(0,0,0,.75));font-size:11px;font-weight:600;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;opacity:0;transition:opacity .2s}
.pick:hover .by{opacity:1}
.pick .flag{position:absolute;top:5px;right:5px;width:24px;height:24px;border-radius:50%;border:0;background:rgba(0,0,0,.6);color:#fff;font-size:12px;cursor:pointer;opacity:0;transition:opacity .2s;display:grid;place-items:center}
.pick:hover .flag{opacity:1}
.pick .flag:hover{background:var(--bad)}
.cempty{color:var(--muted);font-size:14px;padding:18px;border:1px dashed var(--line-hi);border-radius:var(--r-md);text-align:center;margin-top:14px;flex:1}
.mclip{display:none;gap:14px;align-items:center;flex-wrap:wrap;margin-top:16px;padding-top:16px;border-top:1px solid var(--line)}
.mclip.on{display:flex;animation:fadeUp .35s var(--ease)}
.mclip label{color:var(--muted);font-size:13.5px;display:flex;gap:8px;align-items:center}
.mclip input,.mclip select{padding:8px 12px;border-radius:var(--r-sm);border:1px solid var(--line);background:rgba(0,0,0,.3);color:var(--ink);font-size:14px}
.mclip input{width:80px}
.mclip input[type=checkbox]{width:auto;accent-color:#9fd3ff}
.mclip audio{height:38px;flex:1;min-width:240px}
.field{display:grid;gap:8px}
.field label{font-size:13.5px;color:var(--muted)}
.txt{width:100%;padding:13px 16px;border-radius:var(--r-md);border:1px solid var(--line-hi);background:rgba(0,0,0,.3);font-size:15.5px;transition:border-color .2s,box-shadow .2s}
.txt:focus{outline:0;border-color:rgba(63,208,255,.55);box-shadow:0 0 0 4px rgba(63,208,255,.12)}
.log{margin:0;border-radius:var(--r-md);padding:14px 18px 14px 36px;font-size:14px;display:none;animation:fadeUp .3s var(--ease)}
.log li{margin:2px 0}
.log.ok{border-color:rgba(61,220,151,.45)} .log.bad{border-color:rgba(255,107,122,.55);color:#ffd3d8}

/* Sticky action bars */
.actbar{position:sticky;bottom:0;z-index:5;display:flex;align-items:center;gap:12px;padding:14px 28px;border-width:1px 0 0;border-radius:0;box-shadow:0 -10px 40px rgba(0,0,0,.3);background:rgba(8,12,26,.72)}
.actbar .note{color:var(--muted);font-size:13.5px;flex:1;min-width:0}
.pend{display:flex;align-items:center;gap:10px;color:var(--muted);flex:1}
.pend b{width:9px;height:9px;border-radius:50%;background:#58627a;transition:background .3s,box-shadow .3s}
.pend.has{color:var(--ink)} .pend.has b{background:var(--warn);box-shadow:0 0 10px var(--warn)}

/* ── Home layout view ── */
.lv{grid-template-columns:1fr 380px;grid-template-rows:1fr auto;min-height:calc(100vh - var(--top))}
.lv-main{padding:28px;display:flex;flex-direction:column;gap:18px;min-width:0}
.lv-empty{margin:auto;max-width:620px;text-align:center;animation:fadeUp .5s var(--ease)}
.lv-empty h1{font-size:36px;letter-spacing:-.03em;margin:0 0 10px}
.lv-empty p{color:var(--muted);font-size:16px}
.lwide{width:min(100%,calc((100vh - 450px)*16/9));min-width:min(100%,560px);margin:0 auto}
.ltabs{position:absolute;left:4.5cqw;top:2.2cqw;display:flex;gap:3cqw;font-size:1.75cqw;font-weight:600;color:rgba(255,255,255,.6);z-index:2}
.ltabs span{position:relative;padding-bottom:.4cqw;cursor:pointer;transition:color .3s}
.ltabs span::after{content:"";position:absolute;left:0;right:0;bottom:0;height:.22cqw;border-radius:2px;background:#fff;transform:scaleX(0);transition:transform .35s var(--ease)}
.ltabs span.on{color:#fff}
.ltabs span.on::after{transform:scaleX(1)}
/* Tiles are placed one by one so moving the selection slides the row, like the console does. */
.lrow{position:absolute;inset:0;transition:opacity .2s,transform .25s var(--ease)}
.lrow.swap{opacity:0;transform:translateX(1.5cqw)}
.ltile{position:absolute;top:7.2cqw;width:6.2cqw;aspect-ratio:1;border-radius:.9cqw;background:rgba(255,255,255,.13) center/cover;cursor:pointer;
  transition:left .45s var(--ease),width .45s var(--ease),border-radius .45s var(--ease),opacity .35s,outline-color .3s,box-shadow .45s;
  outline:.28cqw solid transparent;outline-offset:.32cqw;animation:tileIn .4s var(--ease) backwards}
@keyframes tileIn{from{opacity:0;transform:translateX(2cqw)}}
.ltile.sel{width:12.5cqw;border-radius:1.3cqw;outline-color:#fff;box-shadow:0 1.2cqw 3cqw rgba(0,0,0,.55)}
.ltile.gone{opacity:0;pointer-events:none}
.ltile i,.lcard .im i{position:absolute;inset:0;display:grid;place-items:center;font-style:normal;font-weight:600;font-size:1.3cqw;color:rgba(255,255,255,.8)}
.ltitle{position:absolute;left:4.5cqw;top:24cqw;font-size:3.2cqw;font-weight:600;text-shadow:0 .3cqw 1.5cqw rgba(0,0,0,.6);max-width:60cqw;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;transition:opacity .18s,transform .25s var(--ease)}
.lsub{position:absolute;left:4.5cqw;top:29.5cqw;font-size:1.4cqw;color:rgba(255,255,255,.72);transition:opacity .18s}
.flip{opacity:0!important;transform:translateY(.8cqw)}
.lbar{display:flex;align-items:center;gap:12px;flex-wrap:wrap}
.lsorts{margin-left:auto;display:flex;align-items:center;gap:6px}
.lsorts label{display:flex;align-items:center;gap:8px;font-size:14px;color:var(--muted);margin-left:8px;cursor:pointer}
.lsorts input{width:17px;height:17px;accent-color:var(--cyan)}
.lstrip{display:flex;gap:18px;overflow-x:auto;padding:6px 4px 18px}
.lgrp{display:flex;flex-direction:column;gap:10px;flex:none}
.lgrp + .lgrp{padding-left:18px;border-left:1px solid var(--line)}
.lgrp>span{font-size:12.5px;font-weight:600;letter-spacing:.06em;text-transform:uppercase;color:var(--dim)}
.lcards{display:flex;gap:12px}
.lcard{flex:none;width:132px;padding:8px;border-radius:var(--r-md);border:1px solid transparent;background:var(--fill);cursor:pointer;position:relative;text-align:left;
  transition:background .2s,border-color .2s,transform .25s var(--spring),box-shadow .25s,opacity .2s}
.lcard:hover{background:var(--fill-hi);transform:translateY(-3px)}
.lcard[aria-current="true"]{border-color:rgba(63,208,255,.6);background:linear-gradient(160deg,rgba(63,208,255,.16),rgba(47,107,255,.08));box-shadow:0 8px 24px rgba(47,107,255,.25)}
.lcard.drag{opacity:.3}
.lcard.over{box-shadow:-6px 0 0 -2px var(--cyan)}
.lcard .im{aspect-ratio:1;border-radius:11px;background:rgba(0,0,0,.3) center/cover;position:relative}
.lcard .im i{font-size:16px}
.lcard .nm{margin-top:8px;font-size:13.5px;font-weight:600;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.lcard .bd{position:absolute;top:14px;right:14px;font-size:11px;font-weight:700;padding:2px 8px;border-radius:999px;background:var(--warn);color:#2a1800}
.lempty{color:var(--muted);padding:26px;border:1px dashed var(--line-hi);border-radius:var(--r-md);flex:1;text-align:center}
.lwarn{padding:12px 16px;border-radius:var(--r-md);background:rgba(255,181,71,.08);border:1px solid rgba(255,181,71,.3);color:#ffd9a0;font-size:14px}
.lside{position:sticky;top:var(--top);align-self:start;min-height:calc(100vh - var(--top) - 74px);border-width:0 0 0 1px;border-radius:0;box-shadow:none;padding:28px 24px;display:flex;flex-direction:column;gap:22px;background:linear-gradient(180deg,rgba(255,255,255,.035),rgba(255,255,255,.01))}
.lside>*{animation:fadeUp .35s var(--ease) backwards}
.lside>*:nth-child(2){animation-delay:40ms}.lside>*:nth-child(3){animation-delay:80ms}.lside>*:nth-child(4){animation-delay:120ms}.lside>*:nth-child(5){animation-delay:160ms}
.lside .hd{display:flex;gap:14px;align-items:center}
.lside .hd .im{width:64px;height:64px;border-radius:14px;background:rgba(0,0,0,.3) center/cover;position:relative;flex:none;box-shadow:0 6px 18px rgba(0,0,0,.4)}
.lside .hd .im i{position:absolute;inset:0;display:grid;place-items:center;font-style:normal;font-weight:600}
.lside h2{font-size:21px;margin:0;letter-spacing:-.01em;line-height:1.25;word-break:break-word}
.lside .id{font-size:13px;color:var(--muted)}
.lsec{display:grid;gap:10px}
.lsec>label,.lsec>.lbl{font-size:13.5px;color:var(--muted)}
.lseg{display:grid;grid-template-columns:repeat(3,1fr);padding:4px;border-radius:999px;background:rgba(0,0,0,.25);border:1px solid var(--line)}
.lseg button{border:0;background:none;padding:10px 0;border-radius:999px;color:var(--muted);cursor:pointer;font-weight:600;transition:background .25s,color .25s}
.lseg button:hover{color:var(--ink)}
.lseg button[aria-pressed="true"]{background:linear-gradient(120deg,rgba(63,208,255,.35),rgba(47,107,255,.45));color:#fff;box-shadow:0 4px 16px rgba(47,107,255,.3)}
.lnote{font-size:13.5px;color:var(--muted)}
.linker{border:0;background:none;color:var(--cyan);padding:0;cursor:pointer;justify-self:start;font-size:14px;font-weight:600}
.lv .actbar{grid-column:1/-1}

/* ── dialogs, toasts, busy ── */
.scrim{position:fixed;inset:0;display:grid;place-items:center;background:rgba(2,4,10,.62);backdrop-filter:blur(6px);-webkit-backdrop-filter:blur(6px);z-index:40;padding:20px;
  opacity:0;visibility:hidden;transition:opacity .25s,visibility 0s .25s}
.scrim.open{opacity:1;visibility:visible;transition:opacity .25s}
.dlg{width:min(580px,100%);max-height:88vh;overflow:auto;border-radius:var(--r-xl);padding:30px;background:linear-gradient(160deg,rgba(26,34,62,.92),rgba(10,14,30,.94));
  transform:translateY(14px) scale(.97);transition:transform .4s var(--spring)}
.scrim.open .dlg{transform:none}
.dlg h3{margin:0 0 8px;font-size:23px;letter-spacing:-.02em}
.dlg p{margin:0 0 14px;color:var(--muted)}
.dlg .li{display:flex;align-items:center;gap:14px;padding:12px 14px;border-radius:var(--r-md);background:var(--fill);border:1px solid var(--line);margin-bottom:8px}
.dlg .li div{flex:1;min-width:0}
.dlg .li small{display:block;color:var(--muted)}
.dlg .acts{display:flex;justify-content:flex-end;gap:10px;margin-top:22px;flex-wrap:wrap}
.dlg pre{white-space:pre-wrap;font-size:12.5px;background:rgba(0,0,0,.3);border:1px solid var(--line);border-radius:var(--r-sm);padding:12px;color:var(--muted);max-height:240px;overflow:auto}
.chk{display:flex;align-items:flex-start;gap:10px;margin:16px 0 10px;cursor:pointer}
.chk input{width:18px;height:18px;margin-top:2px;accent-color:var(--cyan);flex:none}
.x{position:absolute;top:16px;right:16px;width:36px;height:36px;border-radius:50%;border:0;background:var(--fill);color:var(--muted);font-size:20px;cursor:pointer;display:grid;place-items:center;transition:background .2s,color .2s}
.x:hover{background:var(--fill-hi);color:#fff}
.cdlg{width:min(620px,100%);text-align:left}
.cdlg .mk{width:58px;height:58px;filter:drop-shadow(0 10px 24px rgba(47,107,255,.5))}
.steps{list-style:none;margin:22px 0;padding:0;display:grid;gap:12px}
.steps li{display:flex;gap:16px;align-items:flex-start;padding:14px 16px;border-radius:var(--r-md);background:var(--fill);border:1px solid var(--line);animation:fadeUp .4s var(--ease) backwards}
.steps li:nth-child(2){animation-delay:60ms}.steps li:nth-child(3){animation-delay:120ms}
.steps b{width:32px;height:32px;border-radius:50%;display:grid;place-items:center;flex:none;font-size:14px;background:linear-gradient(135deg,var(--cyan),var(--blue));box-shadow:0 4px 14px rgba(47,107,255,.4)}
.steps strong{display:block;font-size:15.5px}
.steps span{color:var(--muted);font-size:14px}
.cform{display:flex;gap:10px}
.cform .txt{flex:1;font-size:18px;letter-spacing:.02em}
.cerr{min-height:22px;margin-top:12px;color:var(--bad);font-size:14px}
.cerr.ok{color:var(--ok)}
.shake{animation:shake .4s}
@keyframes shake{20%,60%{transform:translateX(-6px)}40%,80%{transform:translateX(6px)}}
.guide{margin:14px 0;padding:14px 16px 14px 34px;border-radius:var(--r-md);background:rgba(255,181,71,.07);border:1px solid rgba(255,181,71,.25);color:#ffe2b8;font-size:14px}
.guide li{margin:3px 0}
.shareprev{display:flex;gap:16px;align-items:center;margin:6px 0 4px}
.shareprev img{width:96px;height:96px;border-radius:18px;box-shadow:0 10px 28px rgba(0,0,0,.45)}
.toasts{position:fixed;left:50%;bottom:96px;transform:translateX(-50%);z-index:50;display:grid;gap:10px;justify-items:center;pointer-events:none}
.toast{pointer-events:auto;display:flex;align-items:center;gap:14px;padding:12px 14px 12px 20px;border-radius:999px;font-size:14.5px;background:rgba(18,26,50,.92);animation:toastIn .45s var(--spring)}
.toast.out{opacity:0;transform:translateY(10px);transition:opacity .25s,transform .25s}
@keyframes toastIn{from{opacity:0;transform:translateY(16px) scale(.96)}}
.busyv{position:fixed;inset:0;display:grid;place-items:center;background:rgba(2,4,10,.55);backdrop-filter:blur(4px);z-index:45;opacity:0;visibility:hidden;transition:opacity .2s,visibility 0s .2s}
.busyv.open{opacity:1;visibility:visible;transition:opacity .2s}
.busyv .card{display:flex;align-items:center;gap:14px;padding:18px 26px;border-radius:999px;font-weight:600}
.busyv .spin{width:22px;height:22px}
.hidden{display:none!important}
@media (max-width:1100px){.lv{grid-template-columns:1fr}.lside{position:static;min-height:0;border-width:1px 0 0}}
@media (max-width:900px){.art{grid-template-columns:1fr}.rail{position:static;height:auto;max-height:45vh;border-width:0 0 1px}.top{gap:12px;padding:0 14px}.connpill .sub{display:none}.seg button{padding:8px 14px}}
@media (prefers-reduced-motion:reduce){*,*::before{animation:none!important;transition:none!important}}
</style>
</head>
<body>
<header class="top glass">
  <div class="brand"><img src="/wordmark.png" alt="PSGFX"><span class="ver" id="ver">2.0</span></div>
  <nav class="seg views" id="views" aria-label="View"><span class="ind"></span><button data-v="art" aria-pressed="true">Art</button><button data-v="layout" aria-pressed="false">Home layout</button></nav>
  <button class="connpill" id="connpill" title="Connection to your PS5"><span class="dot" id="dot" aria-hidden="true"></span><span id="connT">Not connected</span><span class="sub" id="connS"></span></button>
</header>

<section class="view art on" id="vArt">
  <nav class="rail glass" aria-label="Apps">
    <div class="search"><input id="q" type="search" placeholder="Search apps and games" autocomplete="off" aria-label="Search apps and games"></div>
    <div class="apps" id="apps"><p class="empty">Connect to your PS5 to see its apps.</p></div>
  </nav>

  <div class="amain">
    <div class="acontent">
      <section class="welcome" id="welcome">
        <img class="mk" src="/mark.svg" alt="">
        <h1 id="wTitle">New art for your homebrew and games.</h1>
        <p id="wText">Connect to your PS5 to get started. Then pick an app or game, drop in images or music, and apply.</p>
        <button class="btn primary" id="wConnect">Connect to PS5</button>
      </section>

      <section id="editor" class="hidden" aria-label="Artwork editor" style="display:contents">
        <div class="ahead"><div><h2 id="eName"></h2><div class="sub" id="eId"></div></div></div>
        <div class="stage" id="stage">
          <img class="bgimg" id="bgimg" alt="">
          <div class="shade"></div>
          <div class="tiles" aria-hidden="true">
            <div class="tile sel"><img id="tileimg" alt=""></div>
            <div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div><div class="tile"></div>
          </div>
          <div class="titlezone"><div class="tname" id="tname"></div></div>
          <div class="play" aria-hidden="true">Play</div>
          <div class="staged-badge" id="badge">Preview · not applied yet</div>
          <div class="stage-hint">Drop an image on the tile or the background, or a song anywhere</div>
          <div class="drop d-bg" data-kind="background">Background</div>
          <div class="drop d-icon" data-kind="icon">Icon</div>
        </div>

        <nav class="seg etabs" id="etabs" aria-label="What to change"><span class="ind"></span>
          <button data-t="icon" aria-pressed="true">Icon <b class="pip" data-pip="icon"></b></button>
          <button data-t="background" aria-pressed="false">Background <b class="pip" data-pip="background"></b></button>
          <button data-t="music" aria-pressed="false">Music <b class="pip" data-pip="music"></b></button>
          <button data-t="name" aria-pressed="false">Name <b class="pip" data-pip="name"></b></button>
        </nav>

        <div class="panel on" data-t="icon">
          <div class="card glass chroma slot" data-kind="icon"><div class="thumb"></div><div class="meta"><div class="t">Icon</div><div class="s">Square, 512 × 512 or larger</div><div class="row"><button class="btn pickf">Choose image</button><button class="btn ghost clear hidden">Remove</button></div></div></div>
          <div class="card glass hidden" id="comm">
            <div class="chead"><div><div class="t">Community icons</div><div class="s" id="commS">Shared by other PSGFX users for this game.</div></div>
              <button class="btn" id="share" disabled title="Choose or drop an icon first, then share it">Share your icon</button></div>
            <div class="pgrid" id="commRow"></div>
          </div>
        </div>
        <div class="panel" data-t="background">
          <div class="card glass chroma slot bgslot" data-kind="background"><div class="thumb"></div><div class="meta"><div class="t">Background</div><div class="s">16:9, ideally 3840 × 2160</div><div class="row"><button class="btn pickf">Choose image</button><button class="btn ghost clear hidden">Remove</button></div></div></div>
        </div>
        <div class="panel" data-t="music">
          <div class="card glass chroma">
            <div class="slot musicslot" data-kind="music"><div class="thumb" aria-hidden="true">&#9835;</div><div class="meta"><div class="t">Music</div><div class="s">MP3, WAV or FLAC. Plays when the tile is selected.</div><div class="row"><button class="btn pickf">Choose song</button><button class="btn ghost clear hidden">Remove</button></div></div></div>
            <div class="mclip" id="mclip">
              <label>Start <input id="mstart" value="0:00" inputmode="numeric" title="Where the clip starts in the song (m:ss)"></label>
              <label>Length <select id="mlen"><option value="15">15 s</option><option value="30">30 s</option><option value="60">60 s</option><option value="0">Longest that fits</option></select></label>
              <label title="Marks the whole clip as a loop. Turn off to let the PS5 decide what happens at the end."><input type="checkbox" id="mloop" checked> Loop</label>
              <audio id="maudio" controls preload="none"></audio>
            </div>
          </div>
        </div>
        <div class="panel" data-t="name">
          <div class="card glass chroma field"><label for="aname">Name on the home screen</label><input class="txt" id="aname" maxlength="80" autocomplete="off"></div>
        </div>
        <ul class="log glass" id="log"></ul>
      </section>
    </div>
    <footer class="actbar glass hidden" id="artbar">
      <span class="note" id="anote">Close the app on your PS5 before applying.</span>
      <button class="btn ghost" id="restore">Restore original</button>
      <button class="btn primary" id="apply" disabled>Apply to PS5</button>
    </footer>
  </div>
</section>

<section class="view lv" id="vLayout" aria-label="Home layout">
  <div class="lv-main">
    <div class="lv-empty" id="lvEmpty">
      <h1>Arrange your home screen.</h1>
      <p>Hide tiles like the PlayStation Store, move apps between Games and Media, rename anything, and put your games in any order. PSGFX backs up the home screen before every change.</p>
      <button class="btn primary" id="lvConnect">Connect to PS5</button>
    </div>
    <div id="lvBody" class="hidden" style="display:contents">
      <div id="lvWarn" class="lwide"></div>
      <div class="stage lwide" id="lstage">
        <img class="bgimg" id="lbgA" alt=""><img class="bgimg" id="lbgB" alt="">
        <div class="shade" style="background:linear-gradient(180deg,rgba(0,0,0,.5) 0%,transparent 30%,transparent 55%,rgba(0,0,0,.72) 100%)"></div>
        <div class="ltabs"><span data-w="games">Games</span><span data-w="media">Media</span></div>
        <div class="lrow" id="lrow"></div>
        <div class="ltitle" id="ltitle"></div>
        <div class="lsub" id="lsub"></div>
      </div>
      <div class="lbar lwide">
        <nav class="seg" id="ltabsBar" aria-label="Tiles on"><span class="ind"></span>
          <button data-w="games">Games <small id="lcG"></small></button>
          <button data-w="media">Media <small id="lcM"></small></button>
          <button data-w="hidden">Hidden <small id="lcH"></small></button>
        </nav>
        <div class="lsorts" id="lsorts">
          <button class="btn sm" data-sort="az" title="Sort A to Z">A–Z</button>
          <button class="btn sm" data-sort="za" title="Sort Z to A">Z–A</button>
          <button class="btn sm" data-sort="recent" title="Back to recently played order">Recent</button>
          <label title="Keeps this order even after you play other games"><input type="checkbox" id="llock"> Lock order</label>
        </div>
      </div>
      <div class="lstrip lwide" id="lstrip"></div>
    </div>
  </div>
  <aside class="lside glass" id="lside"><p class="lnote">Select a tile to change it.</p></aside>
  <footer class="actbar glass">
    <div class="pend" id="lpend"><b></b><span>No changes</span></div>
    <button class="btn ghost" id="lPresets">Presets</button>
    <button class="btn ghost" id="lBackups">Backups</button>
    <button class="btn ghost" id="lReload">Reload</button>
    <button class="btn" id="lDiscard" disabled>Discard</button>
    <button class="btn primary" id="lApply" disabled>Apply to PS5</button>
  </footer>
</section>

<div class="scrim" id="cdlg" role="dialog" aria-modal="true" aria-labelledby="cTitle">
  <div class="dlg glass chroma cdlg" style="position:relative">
    <button class="x" id="cClose" aria-label="Close">×</button>
    <img class="mk" src="/mark.svg" alt="">
    <h3 id="cTitle" style="margin-top:12px">Connect to your PS5</h3>
    <p>Three quick steps and you're in.</p>
    <ol class="steps">
      <li><b>1</b><div><strong>Run PS5 Upload on your PS5</strong><span>Load the PS5 Upload payload, the same way you do to install homebrew.</span></div></li>
      <li><b>2</b><div><strong>Open the PS5 Upload app on this PC</strong><span>Version 6 or newer, connected to your console. Keep it open while you use PSGFX.</span></div></li>
      <li><b>3</b><div><strong>Enter your PS5's IP address</strong><span>The same one PS5 Upload uses. On the PS5 it's under Settings › Network › Connection Status.</span></div></li>
    </ol>
    <form class="cform" id="cf">
      <input class="txt" id="ip" placeholder="192.168.1.50" autocomplete="off" inputmode="decimal" aria-label="PS5 IP address">
      <button class="btn primary" id="cbtn">Connect</button>
    </form>
    <div class="cerr" id="cerr" role="status"></div>
  </div>
</div>
<div class="scrim" id="dlg" role="dialog" aria-modal="true"><div class="dlg glass chroma" id="dlgBody"></div></div>
<div class="busyv" id="busyv"><div class="card glass"><span class="spin"></span><span id="busyT">Working…</span></div></div>
<div class="toasts" id="toasts"></div>
<input type="file" id="file" accept="image/png,image/jpeg" hidden>
<input type="file" id="afile" accept=".mp3,.wav,.flac,.aif,.aiff,audio/*" hidden>

<script>
const $ = s => document.querySelector(s);
const st = { apps: [], sel: null, staged: {}, connected: false, community: '', tab: 'icon' };
const KINDS = ['icon','background','music'];
const HINT = {icon:'Square, 512 × 512 or larger', background:'16:9, ideally 3840 × 2160', music:'MP3, WAV or FLAC. Plays when the tile is selected.'};
const esc = s => String(s ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));

async function api(path, opts={}) {
  const r = await fetch(path, opts);
  const ct = r.headers.get('content-type') || '';
  if (ct.includes('json')) { const j = await r.json(); if (j.ok === false) throw new Error(j.error); return j; }
  return r;
}
function showLog(lines, cls){ const el=$('#log'); el.innerHTML=''; lines.forEach(l=>{const li=document.createElement('li');li.textContent=l;el.appendChild(li)}); el.className='log glass '+cls; el.style.display='block'; }

// Sliding highlight under the pressed button of a segmented control.
function segMove(nav){
  const b = nav.querySelector('button[aria-pressed="true"]'), ind = nav.querySelector('.ind');
  if (!ind) return;
  if (!b || !b.offsetWidth) { ind.style.width = '0'; return; }
  ind.style.left = b.offsetLeft + 'px'; ind.style.width = b.offsetWidth + 'px';
}
const segAll = () => document.querySelectorAll('.seg').forEach(segMove);
window.addEventListener('resize', segAll);
document.fonts?.ready.then(segAll);

function toast(msg, undo){
  const t = document.createElement('div'); t.className = 'toast glass chroma';
  t.innerHTML = `<span>${esc(msg)}</span>` + (undo ? '<button class="btn sm">Undo</button>' : '');
  const close = () => { t.classList.add('out'); setTimeout(() => t.remove(), 260); };
  if (undo) t.querySelector('button').onclick = () => { undo(); close(); };
  $('#toasts').appendChild(t); setTimeout(close, undo ? 6000 : 3500);
}
function fadeText(el, txt){
  if (el.textContent === txt) return;
  el.classList.add('flip'); clearTimeout(el._t);
  el._t = setTimeout(() => { el.textContent = txt; el.classList.remove('flip'); }, 150);
}

// ─────────────────────────── Connecting ───────────────────────────
function setConn(state, sub){
  $('#dot').className = 'dot ' + ({ok:'ok', busy:'busy', bad:'bad'}[state] || '');
  $('#connT').textContent = {ok:'Connected', busy:'Connecting…', bad:'Not connected'}[state] || 'Not connected';
  $('#connS').textContent = sub || '';
}
function openConnect(msg){
  $('#cerr').textContent = msg || ''; $('#cerr').className = 'cerr';
  $('#cdlg').classList.add('open'); setTimeout(() => $('#ip').focus(), 60);
}
const closeConnect = () => $('#cdlg').classList.remove('open');
$('#connpill').onclick = () => openConnect();
$('#wConnect').onclick = () => openConnect();
$('#lvConnect').onclick = () => openConnect();
$('#cClose').onclick = closeConnect;
$('#cdlg').addEventListener('click', e => { if (e.target.id === 'cdlg') closeConnect(); });

async function connect(ip, quiet){
  setConn('busy', ip); $('#cbtn').disabled = true;
  $('#cbtn').innerHTML = '<span class="spin"></span>Connecting'; $('#cerr').textContent = ''; $('#cerr').className = 'cerr';
  try {
    const j = await api('/api/connect', {method:'POST', body: JSON.stringify({ip})});
    st.apps = j.apps; st.connected = true; setConn('ok', ip); renderApps(); updateWelcome();
    L.loaded = false; if (st.view === 'layout') lload();
    if ($('#cdlg').classList.contains('open')) { $('#cerr').textContent = `Connected. Found ${j.apps.filter(a => a.editable).length} apps and games.`; $('#cerr').className = 'cerr ok'; setTimeout(closeConnect, 700); }
  } catch (err) {
    st.connected = false; setConn('bad');
    if (!$('#cdlg').classList.contains('open')) openConnect(err.message);
    else { $('#cerr').textContent = err.message; const d = $('#cdlg .dlg'); d.classList.remove('shake'); void d.offsetWidth; d.classList.add('shake'); }
  } finally { $('#cbtn').disabled = false; $('#cbtn').textContent = 'Connect'; }
}
$('#cf').addEventListener('submit', e => { e.preventDefault(); connect($('#ip').value.trim()); });
function updateWelcome(){
  if (st.connected) { $('#wTitle').textContent = 'Pick an app or game.'; $('#wText').textContent = 'Choose something from the list on the left, then drop images or a song onto the preview.'; $('#wConnect').classList.add('hidden'); }
}

// ─────────────────────────── App list ───────────────────────────
st.showHidden = false;
$('#q').addEventListener('input', () => renderApps());
function renderApps(){
  const box = $('#apps'); box.innerHTML = '';
  if (!st.apps.length) { box.innerHTML = '<p class="empty">No apps found.</p>'; return; }
  const q = $('#q').value.trim().toLowerCase();
  const match = a => !q || (a.title_name || '').toLowerCase().includes(q) || a.title_id.toLowerCase().includes(q);
  let n = 0;
  for (const [k, label] of [['homebrew','Homebrew'],['game','Installed games']]) {
    const list = st.apps.filter(a => a.kind === k && !a.hidden && match(a));
    if (!list.length) continue;
    const h = document.createElement('h2'); h.textContent = label; box.appendChild(h);
    list.forEach(a => box.appendChild(appButton(a, n++)));
  }
  const hid = st.apps.filter(a => a.hidden && match(a));
  if (hid.length) {
    const t = document.createElement('button'); t.className = 'btn sm ghost more';
    t.textContent = st.showHidden ? 'Hide other items' : `Show ${hid.length} other item${hid.length>1?'s':''}`;
    t.onclick = () => { st.showHidden = !st.showHidden; renderApps(); };
    box.appendChild(t);
    if (st.showHidden) {
      const h = document.createElement('h2'); h.textContent = 'System and unknown items'; box.appendChild(h);
      hid.forEach(a => box.appendChild(appButton(a, n++)));
    }
  }
  if (!box.querySelector('.app')) box.insertAdjacentHTML('afterbegin', q ? '<p class="empty">Nothing matches that search.</p>' : '<p class="empty">No homebrew or games found.</p>');
}
function appButton(a, i){
  const b = document.createElement('button');
  b.className = 'app' + (a.editable ? '' : ' locked'); b.dataset.id = a.title_id; b.style.setProperty('--i', Math.min(i, 20));
  b.innerHTML = '<img class="ic" alt="" loading="lazy"><div style="min-width:0"><div class="n"></div><div class="i"></div></div>';
  const im = b.querySelector('img'); im.onerror = () => im.classList.add('none');
  im.src = '/api/icon/' + encodeURIComponent(a.title_id);
  b.querySelector('.n').textContent = a.title_name || a.title_id;
  b.querySelector('.i').textContent = a.editable ? a.title_id : a.title_id + " · can't be edited";
  b.disabled = !a.editable;
  if (st.sel && st.sel.title_id === a.title_id) b.setAttribute('aria-current','true');
  b.onclick = () => select(a);
  return b;
}
const railBtn = id => document.querySelector(`.app[data-id="${CSS.escape(id)}"]`);

// ─────────────────────────── Art editor ───────────────────────────
function setTab(t){
  st.tab = t;
  document.querySelectorAll('#etabs button').forEach(b => b.setAttribute('aria-pressed', b.dataset.t === t));
  document.querySelectorAll('.panel').forEach(p => p.classList.toggle('on', p.dataset.t === t));
  segMove($('#etabs'));
}
document.querySelectorAll('#etabs button').forEach(b => b.onclick = () => setTab(b.dataset.t));

async function select(a){
  document.querySelectorAll('.app').forEach(x => x.removeAttribute('aria-current'));
  railBtn(a.title_id)?.setAttribute('aria-current','true');
  st.sel = a;
  for (const k of KINDS) if (st.staged[k]) { await api('/api/unstage/'+k); }
  st.staged = {};
  $('#welcome').classList.add('hidden'); $('#editor').classList.remove('hidden'); $('#artbar').classList.remove('hidden'); $('#log').style.display='none';
  $('#eName').textContent = a.title_name || a.title_id;
  $('#eId').textContent = a.title_id + (a.kind === 'game' ? ' · installed game' : ' · homebrew');
  const ah = $('.ahead'); ah.style.animation = 'none'; void ah.offsetWidth; ah.style.animation = '';
  $('#tname').textContent = a.title_name || a.title_id;
  $('#aname').value = a.title_name || a.title_id;
  $('#anote').textContent = a.kind === 'game' ? 'Close the game first. The PS5 may put back its original art when the game updates.' : 'Close the app on your PS5 before applying.';
  setTab(st.tab);
  refreshPreview(true);
  loadCommunity(a);
}

const markPick = b => { document.querySelectorAll('.pick').forEach(x => x.setAttribute('aria-pressed', 'false')); b?.setAttribute('aria-pressed', 'true'); };

// ── Community icons (community-server/ on the PSGFX site, saved to GitHub) ──
const cURL = p => st.community.replace(/\/?$/, '/') + p;
async function loadCommunity(a){
  const box = $('#comm'), row = $('#commRow');
  if (!st.community) { box.classList.add('hidden'); return; }
  box.classList.remove('hidden'); row.innerHTML = '<div class="cempty">Loading community icons…</div>';
  let j;
  try { j = await (await fetch(cURL('list.php?title=' + encodeURIComponent(a.title_id)))).json(); if (j.ok === false) throw new Error(j.error); }
  catch (err) { if (st.sel === a) row.innerHTML = `<div class="cempty">Couldn't reach the community library right now.</div>`; return; }
  if (st.sel !== a) return;
  row.innerHTML = '';
  if (!j.icons.length) { row.innerHTML = '<div class="cempty">No community icons for this one yet. Be the first to share one.</div>'; return; }
  j.icons.forEach((ic, i) => {
    const b = document.createElement('button'); b.className = 'pick'; b.style.setProperty('--i', Math.min(i, 20));
    b.title = 'Shared by ' + (ic.by || 'Anonymous'); b.setAttribute('aria-pressed', 'false');
    b.innerHTML = `<span class="by">${esc(ic.by || 'Anonymous')}</span><span class="flag" role="button" title="Report this icon" aria-label="Report this icon">⚑</span>`;
    const im = new Image(); im.onload = () => b.style.backgroundImage = `url('${im.src}')`;
    im.onerror = () => { if (ic.mirror_thumb && im.src !== ic.mirror_thumb) im.src = ic.mirror_thumb; else if (im.src !== ic.url) im.src = ic.url; };
    im.src = ic.thumb || ic.url;
    b.onclick = async e => {
      if (e.target.closest('.flag')) return reportIcon(ic, b);
      try {
        let r = await fetch(ic.url); if (!r.ok && ic.mirror) r = await fetch(ic.mirror);
        if (!r.ok) throw new Error("Couldn't download that icon.");
        const s = await api('/api/stage/icon', {method:'POST', body: await r.arrayBuffer()});
        st.staged.icon = {note: s.note || '', community: true}; markPick(b); refreshPreview(false);
      } catch (err) { toast(err.message); }
    };
    row.appendChild(b);
  });
}
function reportIcon(ic, b){
  dlg(`<h3>Report this icon?</h3><p>Report icons that are offensive, spam, or not made for this game. After a few reports it's hidden until it's reviewed.</p>
    <div class="acts"><button class="btn ghost" data-close>Cancel</button><button class="btn primary" id="rGo">Report</button></div>`);
  $('#rGo').onclick = async () => {
    dlgClose();
    try { const f = new FormData(); f.append('id', ic.id); const r = await (await fetch(cURL('report.php'), {method:'POST', body: f})).json(); if (r.ok === false) throw new Error(r.error);
      b.style.opacity = .35; toast('Thanks. The icon was reported.'); }
    catch (err) { toast("Couldn't send the report: " + err.message); }
  };
}
$('#share').onclick = () => {
  const a = st.sel; if (!a || !st.staged.icon) return;
  let nick = ''; try { nick = localStorage.getItem('psgfx.nick') || ''; } catch {}
  dlg(`<h3>Share your icon</h3>
    <div class="shareprev"><img src="/api/share/icon?t=${Date.now()}" alt=""><div><strong>${esc(a.title_name || a.title_id)}</strong><div class="lnote">${esc(a.title_id)}</div></div></div>
    <p style="margin-top:12px">It goes live right away for everyone who opens this game in PSGFX.</p>
    <ul class="guide">
      <li>Icons only, made for this game or app.</li>
      <li>No nudity, gore, hate symbols or harassment.</li>
      <li>No personal info, ads or links.</li>
      <li>Only share art you made or have permission to share.</li>
      <li>Icons that break these rules are removed, and repeat uploaders are blocked.</li>
    </ul>
    <div class="field"><label for="sNick">Your name (optional, shown with the icon)</label><input class="txt" id="sNick" maxlength="24" value="${esc(nick)}" placeholder="Anonymous"></div>
    <label class="chk"><input type="checkbox" id="sAgree"> <span>I made this icon or have permission to share it, and it follows the guidelines.</span></label>
    <div class="cerr" id="sErr"></div>
    <div class="acts"><button class="btn ghost" data-close>Cancel</button><button class="btn primary" id="sGo" disabled>Share icon</button></div>`);
  $('#sAgree').onchange = e => $('#sGo').disabled = !e.target.checked;
  $('#sGo').onclick = async () => {
    const go = $('#sGo'); go.disabled = true; go.innerHTML = '<span class="spin"></span>Sharing';
    try {
      const nick = $('#sNick').value.trim(); try { localStorage.setItem('psgfx.nick', nick); } catch {}
      const img = await (await fetch('/api/share/icon')).blob();
      const f = new FormData(); f.append('title', a.title_id); f.append('name', nick); f.append('agree', '1'); f.append('image', img, 'icon.png');
      const r = await (await fetch(cURL('upload.php'), {method:'POST', body: f})).json();
      if (r.ok === false) throw new Error(r.error);
      dlgClose(); toast('Shared. Thanks for adding to the library!'); loadCommunity(a);
    } catch (err) { $('#sErr').textContent = err.message === 'Failed to fetch' ? "Couldn't reach the community library. Check your internet connection." : err.message; go.disabled = false; go.textContent = 'Share icon'; }
  };
};

function refreshPreview(loadCurrent){
  const a = st.sel, t = Date.now();
  $('#tileimg').src = st.staged.icon ? '/api/staged/icon?t='+t : '/api/icon/'+encodeURIComponent(a.title_id)+'?t='+t;
  const bg = $('#bgimg');
  if (st.staged.background) { bg.onload = () => bg.classList.add('on'); bg.src = '/api/staged/background?t='+t; }
  else if (loadCurrent) { bg.classList.remove('on'); bg.onload = () => bg.classList.add('on'); bg.onerror = () => bg.classList.remove('on');
    bg.src = '/api/current/'+encodeURIComponent(a.title_id)+'?src='+encodeURIComponent(a.src)+'&t='+t; }
  document.querySelectorAll('.slot').forEach(sl => {
    const k = sl.dataset.kind, s = st.staged[k];
    if (k !== 'music') sl.querySelector('.thumb').style.backgroundImage = s ? `url(/api/staged/${k}?t=${t})` : '';
    sl.querySelector('.clear').classList.toggle('hidden', !s);
    const sEl = sl.querySelector('.s');
    if (s && s.note) { sEl.textContent = s.note; sEl.classList.add('warn'); }
    else if (s) { sEl.textContent = s.summary ? 'Ready to apply: ' + s.summary : 'Ready to apply'; sEl.classList.remove('warn'); }
    else { sEl.textContent = HINT[k]; sEl.classList.remove('warn'); }
  });
  if (!st.staged.icon) markPick(null);
  const m = st.staged.music;
  $('#mclip').classList.toggle('on', !!m);
  if (m && m.fresh) { $('#maudio').src = '/api/staged/music?t='+t; m.fresh = false; }
  if (!m) $('#maudio').removeAttribute('src');
  $('#share').disabled = !st.staged.icon;
  updateApply();
}
const newName = () => { const v = $('#aname').value.trim(); return v && v !== (st.sel.title_name || st.sel.title_id) ? v : null; };
function updateApply(){
  const any = KINDS.some(k => st.staged[k]) || !!newName();
  $('#apply').disabled = !any; $('#badge').classList.toggle('on', any);
  for (const k of KINDS) document.querySelector(`[data-pip="${k}"]`).classList.toggle('on', !!st.staged[k]);
  document.querySelector('[data-pip="name"]').classList.toggle('on', !!newName());
}
$('#aname').addEventListener('input', () => { $('#tname').textContent = $('#aname').value.trim() || st.sel.title_name || st.sel.title_id; updateApply(); });
$('#aname').addEventListener('keydown', e => { if (e.key === 'Enter' && !$('#apply').disabled) $('#apply').click(); });

async function stage(kind, file){
  const sl = document.querySelector(`.slot[data-kind="${kind}"] .s`);
  setTab(kind);
  sl.textContent = kind === 'music' ? 'Converting… this takes a few seconds.' : 'Loading…'; sl.classList.remove('warn');
  try {
    const j = await api('/api/stage/'+kind, {method:'POST', body: await file.arrayBuffer()});
    st.staged[kind] = {note: j.note || '', summary: j.summary || '', fresh: true};
    if (kind === 'music') { $('#mstart').value = fmtTime(j.start); $('#mlen').value = String(j.duration); $('#mloop').checked = j.loop !== false; }
  } catch (err) { sl.textContent = err.message; sl.classList.add('warn'); return; }
  if (kind === 'icon') markPick(null);
  refreshPreview(false);
}

const fmtTime = v => Math.floor(v/60) + ':' + String(Math.floor(v%60)).padStart(2,'0');
const parseTime = v => { const p = String(v).trim().split(':').map(Number); if (p.some(isNaN)) return 0; return p.length > 1 ? p[0]*60 + p[1] : p[0]; };
async function reclip(){
  const sl = document.querySelector('.slot[data-kind="music"] .s');
  sl.textContent = 'Converting…'; sl.classList.remove('warn');
  try {
    const j = await api('/api/music/clip', {method:'POST', body: JSON.stringify({start: parseTime($('#mstart').value), duration: Number($('#mlen').value), loop: $('#mloop').checked})});
    st.staged.music = {note: j.note || '', summary: j.summary || '', fresh: true};
    $('#mstart').value = fmtTime(j.start);
  } catch (err) { sl.textContent = err.message; sl.classList.add('warn'); return; }
  refreshPreview(false);
}
$('#mstart').addEventListener('change', reclip); $('#mlen').addEventListener('change', reclip); $('#mloop').addEventListener('change', reclip);
$('#afile').onchange = e => { const f = e.target.files[0]; if (f) stage('music', f); };
const isAudio = f => /^audio\//.test(f.type) || /\.(mp3|wav|flac|aiff?)$/i.test(f.name);

let pickKind = null;
document.querySelectorAll('.slot').forEach(sl => {
  sl.querySelector('.pickf').onclick = () => {
    if (sl.dataset.kind === 'music') { $('#afile').value=''; $('#afile').click(); return; }
    pickKind = sl.dataset.kind; $('#file').value=''; $('#file').click(); };
  sl.querySelector('.clear').onclick = async () => { await api('/api/unstage/'+sl.dataset.kind); delete st.staged[sl.dataset.kind]; refreshPreview(true); };
});
$('#file').onchange = e => { const f = e.target.files[0]; if (f && pickKind) stage(pickKind, f); };

// Drag onto the replica: which zone is under the pointer decides the slot.
const stageEl = $('#stage');
function zoneAt(x, y){
  const r = stageEl.querySelector('.d-icon').getBoundingClientRect();
  return x>=r.left&&x<=r.right&&y>=r.top&&y<=r.bottom ? 'icon' : 'background';
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
  const f = e.dataTransfer.files[0]; if (f) stage(isAudio(f) ? 'music' : zoneAt(e.clientX, e.clientY), f);
});
window.addEventListener('dragover', e => e.preventDefault()); window.addEventListener('drop', e => e.preventDefault());

$('#apply').onclick = async () => {
  const a = st.sel, art = KINDS.some(k => st.staged[k]), name = newName(), lines = [];
  $('#apply').disabled = true; $('#apply').innerHTML = '<span class="spin"></span>Applying';
  showLog([art ? 'Converting and uploading to '+(a.title_name||a.title_id)+'…' : 'Renaming '+(a.title_name||a.title_id)+'…'], '');
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
      const n = railBtn(a.title_id)?.querySelector('.n'); if (n) n.textContent = j.name;
      $('#aname').value = j.name; $('#tname').textContent = j.name; $('#eName').textContent = j.name;
    }
    showLog(lines, 'ok'); toast('Applied. Restart your PS5 to see it.');
  } catch (err) { showLog([...lines, (art && !artDone ? 'Apply failed: ' : 'Rename failed: ')+err.message], 'bad'); }
  $('#apply').textContent = 'Apply to PS5';
  if (artDone) refreshPreview(true); else updateApply();
};
$('#restore').onclick = async () => {
  const a = st.sel; showLog(['Restoring the original art and music…'], '');
  try {
    const j = await api('/api/restore', {method:'POST', body: JSON.stringify({title_id:a.title_id, src:a.src, kind:a.kind})});
    showLog(j.log, 'ok'); refreshPreview(true);
    refreshRailIcon(a);
  } catch (err) { showLog([err.message], 'bad'); }
};
function refreshRailIcon(a){ const im = railBtn(a.title_id)?.querySelector('.ic'); if (im) { im.classList.remove('none'); im.src = '/api/icon/'+encodeURIComponent(a.title_id)+'?t='+Date.now(); } }

// ─────────────────────────── Home layout ───────────────────────────
const L = { info:null, edits:{}, order:[], base:[], dragged:false, w:'games', sel:null, focus:0, loaded:false, lastPreset:'' };
const LIB = 'NPXS40071';
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
const lstripIds = w => lgroups(w).flatMap(g => g.list.map(t => t.id));
function lrowFor(w){
  const [sys, mine] = lgroups(w).map(g => g.list);
  if (w === 'media') return [...mine, ...sys];
  const lib = sys.filter(t => t.id === LIB), lead = sys.filter(t => t.home && t.id !== LIB), rest = sys.filter(t => !t.home && t.id !== LIB);
  return [...lead, ...mine, ...lib, ...rest];
}
const initials = n => esc(String(n).replace(/[^\p{L}\p{N}\s]/gu, '').split(/\s+/).filter(Boolean).slice(0,2).map(x => x[0]).join('').toUpperCase());
function setImg(el, src, name){
  el.dataset.src = src || ''; el.style.backgroundImage = '';
  el.innerHTML = `<i>${initials(name)}</i>`;
  if (!src) return;
  const im = new Image(); im.onload = () => { if (el.dataset.src === src) { el.style.backgroundImage = `url('${src}')`; el.innerHTML = ''; } }; im.src = src;
}

function lrender(){
  const has = !!L.info;
  $('#lvEmpty').classList.toggle('hidden', has); $('#lvBody').classList.toggle('hidden', !has);
  lfooter();
  if (!has) { $('#lside').innerHTML = '<p class="lnote">Select a tile to change it.</p>'; return; }
  $('#lvWarn').innerHTML = (L.info.warnings || []).map(w => `<div class="lwarn">${esc(w)}</div>`).join('');
  document.querySelectorAll('#ltabsBar button').forEach(b => b.setAttribute('aria-pressed', b.dataset.w === L.w));
  segMove($('#ltabsBar'));
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
      `<button class="lcard" data-id="${esc(t.id)}" data-g="${g.key}" draggable="${g.key === 'mine' && L.info.canOrder}" ${t.id === L.sel ? 'aria-current="true"' : ''}>
        <div class="im" data-src="${esc(limg(t.icon))}" data-n="${esc(t.name)}"></div>${t.changed ? '<span class="bd">Edited</span>' : ''}
        <div class="nm" title="${esc(t.name)}">${esc(t.name)}</div></button>`).join('')}</div></div>`).join('');
    strip.querySelectorAll('.im').forEach(el => setImg(el, el.dataset.src, el.dataset.n));
    strip.querySelector('[aria-current="true"]')?.scrollIntoView({block:'nearest', inline:'nearest', behavior:'smooth'});
  }
  lpreview();
  lside();
}

// The replica: the selected tile sits at the left like on the console, and the row slides as it changes.
function lpreview(){
  const pw = L.w === 'hidden' ? 'games' : L.w;
  document.querySelectorAll('.ltabs span').forEach(s => s.classList.toggle('on', s.dataset.w === pw));
  const row = lrowFor(pw);
  if (L.sel) { const i = row.findIndex(t => t.id === L.sel); if (i >= 0) L.focus = i; }
  L.focus = Math.max(0, Math.min(L.focus, row.length - 1));
  const el = $('#lrow');
  if (el.dataset.w && el.dataset.w !== pw) {
    el.classList.add('swap'); clearTimeout(el._t);
    el._t = setTimeout(() => { el.innerHTML = ''; el.dataset.w = pw; lplace(row); el.classList.remove('swap'); }, 200);
  } else { el.dataset.w = pw; lplace(row); }
  const f = row[L.focus];
  fadeText($('#ltitle'), f ? f.name : 'Nothing on this tab');
  fadeText($('#lsub'), f && f.kind === 'system' ? 'System' : '');
  crossBg(f && f.bg ? limg(f.bg) : '');
}
const SMALL = 6.2, BIG = 12.5, GAP = .9, LEFT = 4.5;
function lplace(row){
  const el = $('#lrow'), keep = new Map([...el.children].map(c => [c.dataset.id, c])), seen = new Set(), f = L.focus;
  for (let i = Math.max(0, f - 2); i < Math.min(row.length, f + 14); i++) {
    const t = row[i], d = i - f;
    const left = d < 0 ? LEFT + d * (SMALL + GAP) : d === 0 ? LEFT : LEFT + BIG + GAP * 1.6 + (d - 1) * (SMALL + GAP);
    let c = keep.get(t.id);
    if (!c) {
      c = document.createElement('div'); c.className = 'ltile'; c.dataset.id = t.id;
      c.style.transition = 'none'; c.style.left = left + 'cqw';
      el.appendChild(c); void c.offsetWidth; c.style.transition = '';
    }
    if (c.dataset.src !== limg(t.icon) || c.dataset.n !== t.name) { c.dataset.n = t.name; setImg(c, limg(t.icon), t.name); }
    c.style.left = left + 'cqw';
    c.classList.toggle('sel', d === 0); c.classList.toggle('gone', d < 0);
    seen.add(t.id);
  }
  for (const [id, c] of keep) if (!seen.has(id)) c.remove();
}
let bgFront = 'A';
function crossBg(src){
  const front = $('#lbg' + bgFront), back = $('#lbg' + (bgFront === 'A' ? 'B' : 'A'));
  if ((front.dataset.src || '') === src) return;
  if (!src) { front.classList.remove('on'); front.dataset.src = ''; return; }
  back.onload = () => { if (back.dataset.src !== src) return; back.classList.add('on'); front.classList.remove('on'); bgFront = bgFront === 'A' ? 'B' : 'A'; };
  back.onerror = () => { front.classList.remove('on'); };
  back.dataset.src = src; front.dataset.src = ''; back.src = src;
}

function lside(){
  const box = $('#lside'), t0 = L.sel && lbyId(L.sel);
  if (!t0) { box.innerHTML = '<p class="lnote">Select a tile to change it.</p>'; return; }
  const t = leff(t0), e = L.edits[t.id] || {};
  const art = st.apps.find(a => a.title_id === t.id && a.editable);
  box.innerHTML = `<div class="hd"><div class="im" id="lsImg"></div><div style="min-width:0"><h2>${esc(t.name)}</h2><div class="id">${esc(t.id)}${t.kind === 'system' ? ' · system tile' : ''}</div></div></div>
    <div class="lsec"><span class="lbl">Shows on</span><div class="lseg">${['games','media','hidden'].map(v =>
      `<button data-where="${v}" aria-pressed="${t.where === v}">${v[0].toUpperCase() + v.slice(1)}</button>`).join('')}</div></div>
    <div class="lsec"><label for="lname">Name on the home screen</label><input class="txt" id="lname" value="${esc(t.name)}" maxlength="80">
      ${e.name != null ? '<button class="linker" id="lname0">Use original name</button>' : ''}</div>
    ${art ? '<div class="lsec"><span class="lbl">Icon, background and music</span><button class="btn" id="lart" style="justify-self:start">Edit in Art</button></div>' : ''}
    ${t.changed ? '<button class="linker" id="lundo" style="color:var(--bad)">Undo changes to this tile</button>' : ''}`;
  setImg($('#lsImg'), limg(t.icon), t.name);
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
}
// Moving a tile off the tab you're looking at selects the next one, so you can keep going down the list.
function lmove(id, where){
  const before = lstripIds(L.w), idx = before.indexOf(id), prev = L.edits[id] ? {...L.edits[id]} : null, name = leff(lbyId(id)).name;
  lsetEdit(id, 'where', where);
  const after = lstripIds(L.w);
  if (idx >= 0 && !after.includes(id)) {
    L.sel = after[Math.min(idx, after.length - 1)] ?? null;
    toast(where === 'hidden' ? `Hid ${name}` : `Moved ${name} to ${where[0].toUpperCase() + where.slice(1)}`, () => {
      if (prev) L.edits[id] = prev; else delete L.edits[id];
      L.sel = id; lrender();
    });
  }
  lrender();
}
function lreset(){
  L.order = ltiles().filter(t => t.kind !== 'system').sort((a,b) => b.access - a.access).map(t => t.id);
  L.base = [...L.order]; L.edits = {}; L.dragged = false; $('#llock').checked = false;
  if (L.sel && !lbyId(L.sel)) L.sel = null;
}

function busyv(on, t){ $('#busyv').classList.toggle('open', on); if (t) $('#busyT').textContent = t; }
function dlg(html){ $('#dlgBody').innerHTML = html; $('#dlg').classList.add('open'); }
function dlgClose(){ $('#dlg').classList.remove('open'); }
$('#dlg').addEventListener('click', e => { if (e.target.id === 'dlg' || e.target.closest('[data-close]')) dlgClose(); });
document.addEventListener('keydown', e => { if (e.key === 'Escape') { dlgClose(); if (st.connected) closeConnect(); } });

async function lload(){
  busyv(true, 'Reading your home screen…');
  try { L.info = await api('/api/layout/load', {method:'POST', body: JSON.stringify({})}); lreset(); L.loaded = true; if (!L.sel) L.sel = lstripIds(L.w)[0] ?? null; }
  catch (err) { L.info = null; $('#lvEmpty').innerHTML = `<h1>Couldn't read the home screen</h1><p>${esc(err.message)}</p>`; }
  busyv(false); lrender();
}

// views
function setView(v){
  st.view = v;
  document.querySelectorAll('#views button').forEach(b => b.setAttribute('aria-pressed', b.dataset.v === v));
  segMove($('#views'));
  $('#vArt').classList.toggle('on', v === 'art');
  $('#vLayout').classList.toggle('on', v === 'layout');
  if (v === 'layout' && !L.loaded && st.connected) lload();
  lrender(); segAll();
}
st.view = 'art';
document.querySelectorAll('#views button').forEach(b => b.onclick = () => setView(b.dataset.v));

// interactions
function lsetTab(w){ L.w = w; L.focus = 0; L.sel = lstripIds(w)[0] ?? null; lrender(); }
document.querySelectorAll('#ltabsBar button').forEach(b => b.onclick = () => lsetTab(b.dataset.w));
document.querySelectorAll('.ltabs span').forEach(s => s.onclick = () => lsetTab(s.dataset.w));
$('#lstrip').addEventListener('click', e => { const c = e.target.closest('.lcard'); if (c) { L.sel = c.dataset.id; lrender(); } });
$('#lrow').addEventListener('click', e => { const c = e.target.closest('.ltile'); if (c) { L.sel = c.dataset.id; lrender(); } });
document.addEventListener('keydown', e => {
  if (st.view !== 'layout' || !L.info || /INPUT|TEXTAREA|SELECT/.test(document.activeElement.tagName) || $('#dlg').classList.contains('open')) return;
  if (e.key !== 'ArrowLeft' && e.key !== 'ArrowRight') return;
  const row = lrowFor(L.w === 'hidden' ? 'games' : L.w); if (!row.length) return;
  L.focus = Math.max(0, Math.min(row.length - 1, L.focus + (e.key === 'ArrowRight' ? 1 : -1)));
  L.sel = row[L.focus].id; lrender(); e.preventDefault();
});
$('#lside').addEventListener('click', e => {
  const id = L.sel; if (!id) return;
  const w = e.target.closest('[data-where]'); if (w) { if (w.getAttribute('aria-pressed') !== 'true') lmove(id, w.dataset.where); return; }
  if (e.target.closest('#lname0')) { lsetEdit(id, 'name', null); return lrender(); }
  if (e.target.closest('#lundo')) { delete L.edits[id]; return lrender(); }
  if (e.target.closest('#lart')) {
    const a = st.apps.find(x => x.title_id === id); if (!a) return;
    setView('art');
    if (a.hidden && !st.showHidden) st.showHidden = true;
    $('#q').value = ''; renderApps(); select(a);
    railBtn(id)?.scrollIntoView({block:'center', behavior:'smooth'});
  }
});
$('#lside').addEventListener('change', e => { if (e.target.id === 'lname') { lsetEdit(L.sel, 'name', e.target.value.trim()); lrender(); } });
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
    <label class="chk"><input type="checkbox" id="dSave" checked> <span>Also save this layout as a preset</span></label>
    <input class="txt" id="dName" value="${esc(L.lastPreset || 'My layout')}" maxlength="60" aria-label="Preset name">
    <p class="lnote" style="margin-top:8px">Re-apply it in one click after a firmware update or database rebuild resets your home screen.</p>
    <div class="acts"><button class="btn ghost" data-close>Cancel</button><button class="btn primary" id="dGo">Apply changes</button></div>`);
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
  dlg(`<h3>Home screen backups</h3><p>Saved in the "PSGFX Layout" folder (next to PSGFX.exe on Windows, in ~/Library/Application Support/PSGFX on Mac), before every change, plus one of the very first home screen PSGFX read.</p>
    ${j.backups.length ? j.backups.map(b => `<div class="li"><div>${esc(b.label)}<small>${esc(b.created)} · ${esc(b.ps5)}</small></div><button class="btn sm" data-restore="${esc(b.id)}" ${st.connected ? '' : 'disabled'}>Restore</button></div>`).join('') : '<p class="lnote">No backups yet. One is made the first time you open Home layout.</p>'}
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
    ${j.presets.length ? j.presets.map(n => `<div class="li"><div>${esc(n)}</div><button class="btn sm" data-load="${esc(n)}">Load</button></div>`).join('') : '<p class="lnote">No presets yet. Tick "Also save this layout as a preset" when you apply.</p>'}
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

segAll();
api('/api/config').then(j => {
  if (j.version) $('#ver').textContent = j.version;
  st.community = j.community || '';
  if (j.ip) { $('#ip').value = j.ip; connect(j.ip, true); }
  else openConnect();
});
</script>
</body>
</html>
)HTML";
