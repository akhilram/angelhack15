/*
 * pebble-js-app.js
 * Runs a counter that increments a variable and sends the new value 
 * to Pebble for processing in sync_changed_handler().
 */
var count = 0;
var response;
var referesh_interval_id;
var emit = function() {
    if (count < response.length) {
        //var text = response[count];
        var dict = {
            "0": response[count]
        };
        console.log("Sending message:" + response[count]);
        Pebble.sendAppMessage(dict);
        count += 1;
    } else {
//        clearInterval(referesh_interval_id);
    }
};

Pebble.addEventListener('ready', function(e) {
    console.log('PebbleKit JS ready!');
    getJson(3);
});

function getJson(selection) {
    var req = new XMLHttpRequest();
    var baseurl = "http://sudotestapp-env.elasticbeanstalk.com/index.php?";
    var topic = "category=topStories";
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
    response = ["fetching data"];
    req.send();
    req.onload = function(e) {
        console.log(req.status);
        if (req.readyState == 4) {
            if (req.status == 200) {
                count = 0;
                console.log(req.responseText);
                response = [];
                //response = req.responseText.split("\",\"");
                response = JSON.parse(req.responseText);

                // Send periodic updates every 3 seconds
                referesh_interval_id = setInterval(emit, 625);
            }
        }
    };
}

Pebble.addEventListener('appmessage', function(e) {
    console.log('AppMessage received!');
    console.log('payload ' + JSON.stringify(e.payload));
    getJson(e.payload.status);
});
