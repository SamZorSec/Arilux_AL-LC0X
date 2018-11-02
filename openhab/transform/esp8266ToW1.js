// Converts esp8266 state to white value for openhab
(function(i){
    regex = /(hsb=)([,.\d]+)/;
    m=null;
    if ((m = regex.exec(i)) !== null) {
        hsbw1w2 = m[2].split(',');
        return parseFloat(hsbw1w2[3]);
    }
    return -1.0;
})(input);

