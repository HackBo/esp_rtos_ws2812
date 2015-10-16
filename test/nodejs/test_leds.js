const coap  = require('coap');

nleds=60
leds=new Buffer(nleds*3);

req   = coap.request('coap://192.168.50.1/nleds')
req.write(nleds.toString());
req.on('response', function(res) {
   res.pipe(process.stdout)
})

req.end()


j=0;
setInterval(function() {
  leds.fill(0);
  req   = coap.request('coap://192.168.50.1/rgb')

  j=(j+1)%nleds;
  for(var i=0;i<nleds;i++)
  {
    leds[i*3]=i;
    leds[i*3+2]=nleds-i;    

    if (i==j)
    {
      leds[i*3+0]=255;
      leds[i*3+1]=255;  
      leds[i*3+2]=255;    
    }
  }
  
  req.write(leds.toString('hex'));

  req.on('response', function(res) {
      res.pipe(process.stdout)
  })

  req.end()
},  10);



