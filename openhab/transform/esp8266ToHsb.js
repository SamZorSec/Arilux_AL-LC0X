// Converts esp8266 state to HSB value for openHab
(function(i){
    i=i.toLowerCase();
    regex = /(hsb=)([,.\d]+)/;
    m=null;
    if ((m = regex.exec(i)) !== null) {
        hsbw1w2 = m[2].split(',');
        return hsbw1w2.slice(0,3).join(',');
    }
    return i;
})(input);
