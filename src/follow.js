/**
 * Welcome to Pebble.js!
 *
 * This is where you write your app.
 */

var UI = require('ui');
var Vector2 = require('vector2');

var main = new UI.Card({
  title: 'Pebble F(ol)low',
  icon: 'images/menu_icon.png',
  subtitle: 'Hello World!',
  body: 'Press up button for menu.'
});

main.show();

main.on('click', 'up', function(e) {
  var menu = new UI.Menu({
    sections: [{
      items: [{
        title: 'Sports',
      }, {
        title: 'Science',
      }]
    }]
  });
  menu.on('select', function(e) {
    console.log('Selected item #' + e.itemIndex + ' of section #' + e.sectionIndex);
    console.log('The item is titled "' + e.item.title + '"');
    
    var req = new XMLHttpRequest();
  req.open('GET', "http://sudotestapp-env.elasticbeanstalk.com/index.php", true);
  req.send();
  req.onload = function(e) {
    console.log(req.status);
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var wind = new UI.Window({
          fullscreen:true,
        });
        
        var textfield = new UI.Text({
           position: new Vector2(0, 65),
           size: new Vector2(144, 30),
           font: 'gothic-24-bold',
           text: response.item,
           textAlign: 'center'
        });
        
        wind.add(textfield);
        wind.show();
   
      }
    }
  };
    
  });
  menu.show();
});



