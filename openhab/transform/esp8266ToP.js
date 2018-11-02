// Converts esp8266 state to power for openhab
(function(i){
    i=i.toLowerCase();
    if (i.indexOf('off') !== -1) return 'OFF';
    if (i.indexOf('on') !== -1) return 'ON';
    return 'error';
})(input);

