// Converts esp8266 state to brightness for openhab
(function(i){
    regex = /(hsb=)([,.\d]+)/;
    m=null;
    if ((m = regex.exec(i)) !== null) {
        hsbw1w2 = m[2].split(',');
        return parseFloat(hsbw1w2[2]);
    }
    return -1.0;
})(input);

