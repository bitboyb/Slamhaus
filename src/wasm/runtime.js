const modules=new Map();
const encoder=new TextEncoder();
const state=globalThis.__slamhaus??={input:{keys:new Set(),pressed:new Set(),released:new Set(),buttons:new Set(),touches:new Map(),x:0,y:0,dx:0,dy:0,wheelX:0,wheelY:0}};

function moduleFor(url,canvas){
  if(!modules.has(url))modules.set(url,import(url).then(({default:init})=>init({canvas})).catch(error=>{modules.delete(url);throw error}));
  return modules.get(url);
}

function elementValue(selector){
  const element=document.querySelector(selector);
  if(!element)return '';
  if(element instanceof HTMLInputElement&&element.type==='checkbox')return element.checked?'true':'false';
  return String(element.value??element.textContent??'');
}

function payload(arguments_=[]){
  return arguments_.map(argument=>{const value=typeof argument==='string'?elementValue(argument):String(argument.literal??'');return `${encoder.encode(value).length}:${value}`}).join('');
}

function write(selector,value,error=false){
  if(!selector)return;
  const element=document.querySelector(selector);
  if(!element)return;
  if('value'in element)element.value=value;else element.textContent=value;
  element.toggleAttribute('data-slamhaus-error',error);
}

async function bind(options){
  const run=async event=>{
    if(event?.type==='submit')event.preventDefault();
    try{
      const module=await moduleFor(options.module);
      const result=module.ccall('slam_invoke','string',['string','string'],[options.function,payload(options.args)]);
      const failed=result.startsWith('1:');
      write(options.output,result.slice(2),failed);
      if(failed)console.error(`Slamhaus: ${result.slice(2)}`);
    }catch(error){write(options.output,String(error),true);console.error('Slamhaus module failed',error)}
  };
  if(options.target){
    const target=document.querySelector(options.target);
    if(!target)throw new Error(`Missing binding target ${options.target}`);
    target.addEventListener(options.event||'click',run);
  }else await run();
}

function installInput(canvas){
  if(state.installed)return;
  state.installed=true;state.canvas=canvas;
  const input=state.input;
  addEventListener('keydown',event=>{if(!input.keys.has(event.keyCode))input.pressed.add(event.keyCode);input.keys.add(event.keyCode)});
  addEventListener('keyup',event=>{input.keys.delete(event.keyCode);input.released.add(event.keyCode)});
  addEventListener('pointerdown',event=>input.buttons.add(event.button));
  addEventListener('pointerup',event=>input.buttons.delete(event.button));
  addEventListener('pointermove',event=>{const rect=canvas.getBoundingClientRect();input.x=event.clientX-rect.left;input.y=event.clientY-rect.top;input.dx+=event.movementX;input.dy+=event.movementY});
  addEventListener('wheel',event=>{input.wheelX+=event.deltaX;input.wheelY+=event.deltaY},{passive:true});
  for(const name of ['touchstart','touchmove','touchend','touchcancel'])canvas.addEventListener(name,event=>{input.touches.clear();for(const touch of event.touches)input.touches.set(touch.identifier,{x:touch.clientX,y:touch.clientY})},{passive:true});
  const sensors=state.sensors??={};
  addEventListener('deviceorientation',event=>{sensors.alpha=event.alpha;sensors.beta=event.beta;sensors.gamma=event.gamma});
  addEventListener('devicemotion',event=>{sensors.ax=event.acceleration?.x;sensors.ay=event.acceleration?.y;sensors.az=event.acceleration?.z});
}

async function application(options){
  const canvas=document.querySelector(options.canvas);
  if(!(canvas instanceof HTMLCanvasElement))throw new Error(`Missing application canvas ${options.canvas}`);
  installInput(canvas);
  const module=await moduleFor(options.module,canvas);
  const call=name=>module.ccall(name,null,[],[]);
  const resize=()=>{const rect=canvas.getBoundingClientRect(),scale=devicePixelRatio||1;canvas.width=Math.max(1,Math.round(rect.width*scale));canvas.height=Math.max(1,Math.round(rect.height*scale));call('slam_resize_application')};
  new ResizeObserver(resize).observe(canvas);resize();call('slam_start_application');
  let previous=performance.now(),active=true;
  const frame=now=>{if(!active)return;const delta=Math.min(.25,(now-previous)/1000);previous=now;module.ccall('slam_tick',null,['number'],[delta]);const input=state.input;input.pressed.clear();input.released.clear();input.dx=input.dy=input.wheelX=input.wheelY=0;requestAnimationFrame(frame)};
  requestAnimationFrame(frame);
  document.addEventListener('visibilitychange',()=>module.ccall('slam_suspend_application',null,['number'],[document.hidden?1:0]));
  addEventListener('pagehide',()=>{active=false;call('slam_stop_application')},{once:true});
}

const descriptors=[...document.querySelectorAll('[data-slamhaus-bindings]')].flatMap(node=>JSON.parse(node.textContent));
for(const options of descriptors)try{if(options.application)await application(options);else await bind(options)}catch(error){console.error('Slamhaus setup failed',error)}

let installPrompt;
addEventListener('beforeinstallprompt',event=>{event.preventDefault();installPrompt=event;state.canInstall=true});
state.install=async()=>{if(!installPrompt)return false;await installPrompt.prompt();installPrompt=undefined;state.canInstall=false;return true};
