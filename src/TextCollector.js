/*
 * pebble-js-app.js
 * Runs a counter that increments a variable and sends the new value 
 * to Pebble for processing in sync_changed_handler().
 */

var count = 0;
var response;
var emit = function() {
  
  //var text = response[count];
  var dict = {"0": response[count]};
  console.log("Sending message:"+response[count]);
  Pebble.sendAppMessage(dict);
  count += 1;
};

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
  var req = new XMLHttpRequest();
  var baseurl = "http://sudotestapp-env.elasticbeanstalk.com/index.php?category=topStories";
  
    req.open('GET', baseurl, true);
    console.log(baseurl);
    
    req.send();
    req.onload = function(e) {
      console.log(req.status);
      if (req.readyState == 4) {
        if(req.status == 200) {
          console.log(req.responseText);
          response = [];
          //response = req.responseText.split("\",\"");
          response = JSON.parse(req.responseText);
          
          // Send periodic updates every 3 seconds
          setInterval(emit, 2000);
        }
      }
    };

  
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
});

