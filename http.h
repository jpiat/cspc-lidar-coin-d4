const char index_html[] PROGMEM = R"=====(
<head>
<title> WebSockets Client</title>
<body>
<div>
<canvas id="myCanvas" width="640" height="640" style="border:1px solid black;">
</div>
<div>
<button onclick="start()">Start</button>
</div>
<div id="scanFreq">

</div>
<script>

const start = () => {
  const max_dist = 8000 ;
  const canvas = document.getElementById("myCanvas");
  const freqDiv = document.getElementById("scanFreq");
  const ctx = canvas.getContext("2d");
  ctx.fillStyle = "red";
  var host = `ws://${window.location.hostname}:81`;
  var socket = new WebSocket(host);
  socket.binaryType = 'arraybuffer';
  var pts = [];
  var canvas_draw = document.createElement('canvas');
  canvas_draw.width = canvas.width;
  canvas_draw.height = canvas.height;
  var context_draw = canvas_draw.getContext('2d');
  var last_ts = Date.now();
  socket.onmessage = (msg) => {
      try{
        var dot_array = new Float32Array(msg.data);
        for(var i = 0 ; i < dot_array.length ; i +=3){
            pcm = dot_array.slice(i, i + 3)
            if(pcm[2] > 100 && pcm[1] > 100 && pcm[1] < max_dist){
              const x_scale = canvas.width/max_dist ;
              const y_scale = canvas.height/max_dist ;
              const x = canvas.width/2 - pcm[1] * x_scale * Math.sin(pcm[0] * Math.PI/180.0) ;
              const y = canvas.height/2  - pcm[1] * y_scale * Math.cos(pcm[0] * Math.PI/180.0) ;
              pts.push({a: pcm[0], x : x, y : y});
          }
        }
        context_draw.clearRect(0, 0, canvas_draw.width, canvas_draw.height);
        for (let pt of pts) {
            context_draw.fillRect(pt.x, pt.y, 3,3);
        }
        ctx.clearRect(0, 0, canvas_draw.width, canvas_draw.height);
        ctx.drawImage(canvas_draw, 0, 0);
        pts = []
        var ts = Date.now() ;
        var diff = ts - last_ts ;
        last_ts = ts ;
        var freq = 1/(diff/1000.);
        freqDiv.innerHTML = 'scan frequency: '+freq.toFixed(2)+'Hz';
        
      }catch(e){
        console.log(`Error {e}`);
      }
  };
};
</script>
</body>
</html>

)=====";