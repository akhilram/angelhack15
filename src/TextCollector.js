/*
 * pebble-js-app.js
 * Runs a counter that increments a variable and sends the new value 
 * to Pebble for processing in sync_changed_handler().
 */

var count = 0;
var response;
var emit = function() {
  if (count < response.length){
  //var text = response[count];
  var dict = {"0": response[count]};
  console.log("Sending message:"+response[count]);
  Pebble.sendAppMessage(dict);
    count += 1;}
  else
    {
      return;
    }
};

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
  getJson(1);
});

function getJson(selection){
  var req = new XMLHttpRequest();
  var baseurl = "http://sudotestapp-env.elasticbeanstalk.com/index.php?";
  var topic ="category=topStories";
 switch (selection) {
    case 0:
     topic = "category=topStories";
      break;
    case 1: 
     topic = "category=mostPopular";
      break;
    case 2: 
     topic = "category=finance";
      break;
    case 3: 
     topic = "category=twitter";
      break;
  }
  
    req.open('GET', baseurl.concat(topic), true);
    console.log(baseurl.concat(topic));
    
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
}

function sendMessage() {
	Pebble.sendAppMessage({"status": 0});
}

Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
  console.log('payload '+ e.payload.dummy);
  getJson(e.payload.dummy);
  sendMessage();
});

