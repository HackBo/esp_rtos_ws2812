const coap  = require('coap');

req   = coap.request('coap://192.168.50.1/rgbcolor')
leds=new Buffer(60*3);

leds.fill(0);

for(var i=0;i<60;i++)
{
	leds[i*3]=255;
	leds[i*3+1]=i;
	leds[i*3+2]=i*2;  
}

req.write(leds.toString('hex'));

req.on('response', function(res) {
	res.pipe(process.stdout)
})

req.end()

