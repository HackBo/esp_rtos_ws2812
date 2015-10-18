const coap  = require('coap');

nleds=36
leds=new Buffer(nleds*3);

var c = new Array();

c[0]  = [0,0xFF,0xA2,0x00];
c[1]  = [3,0xFF,0xFE,0x00];
c[2]  = [6,0xD1,0xFF,0x02];
c[3]  = [9,0x76,0xFF,0x00];
c[4]  = [13,0x00,0xFF,0x18];
c[5]  = [15,0x00,0xFF,0xD5];
c[6]  = [18,0x00,0xBD,0xFF];
c[7]  = [21,0x00,0x4B,0xFF];
c[8]  = [24,0x3D,0x00,0xFF];
c[9]  = [27,0xA3,0x01,0xFF];
c[10] = [30,0xFF,0x00,0xEC];
c[11] = [nleds-1,0xFF,0x00,0x00];


for (var i=0; i<c.length-1; i++)
{

  var j=0;
  var dR=(c[i+1][1] - c[i][1])/(c[i+1][0]-c[i][0]);
  var dG=(c[i+1][2] - c[i][2])/(c[i+1][0]-c[i][0]);
  var dB=(c[i+1][3] - c[i][3])/(c[i+1][0]-c[i][0]);

  for (var x=c[i][0]; x<=c[i+1][0]; x++)
  {

      leds[x*3+0]=Math.round(c[i][1] + dR*j);
      leds[x*3+1]=Math.round(c[i][2] + dG*j);
      leds[x*3+2]=Math.round(c[i][3] + dB*j);
      j++;
      console.log( leds[x*3+0] +', ' + leds[x*3+1] +', ' + leds[x*3+2] );
  }
}


req   = coap.request('coap://192.168.50.1/nleds')
req.write(nleds.toString());
req.on('response', function(res) {
   res.pipe(process.stdout)
})

req.end()


req   = coap.request('coap://192.168.50.1/rgb')
req.write(leds.toString('hex'));

req.on('response', function(res) {
   res.pipe(process.stdout)
})

req.end()




setInterval(function() {
  req   = coap.request('coap://192.168.50.1/shift')

  offset=-1

  req.write(offset.toString());


  req.on('response', function(res) {
      res.pipe(process.stdout)
  })

  req.end()
},  20);




