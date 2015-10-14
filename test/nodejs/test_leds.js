const coap  = require('coap');

nleds=60
leds=new Buffer(nleds*3);

j=0;
setInterval(function() {
  leds.fill(0);
  req   = coap.request('coap://192.168.50.1/rgbcolor')

  j=(j+1)%nleds;
  for(var i=0;i<nleds;i++)
  {
    leds[i*3]=0;
    leds[i*3+1]=0;
    leds[i*3+2]=0;  
    if (i==j)
      leds[i*3+2]=55;  
  }
  req.write(leds.toString('hex'));

  req.on('response', function(res) {
      res.pipe(process.stdout)
  })

  req.end()
},  10);



