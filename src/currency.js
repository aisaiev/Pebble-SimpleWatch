function HTTPGET(url) {
	var req = new XMLHttpRequest();
	req.open("GET", url, false);
	req.send(null);
	return req.responseText;
}

var getcurrency = function() {
	//Get weather info
	var response = HTTPGET("https://api.privatbank.ua/p24api/pubinfo?json&exchange&coursid=5");
		
	//Convert to JSON
	var json = JSON.parse(response);
	
	//Extract the data
	var buy = json[2].buy;
	var sale = json[2].sale;
	
	//Console output to check all is working.
  console.log("Buy:" + buy + "Sale:" + sale);
	
	//Construct a key-value dictionary	
	var dict = {"KEY_SALE" : sale, "KEY_BUY": buy};
	
	//Send data to watch for display
	Pebble.sendAppMessage(dict,
    function(e) {
      console.log("Currency info sent to Pebble successfully!");
    },
    function(e) {
      console.log("Error sending currency info to Pebble!");
    }
   );
};

Pebble.addEventListener("ready",
  function(e) {
	//App is ready to receive JS messages
	getcurrency();
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
	//Watch wants new data!
	getcurrency();
  }
);