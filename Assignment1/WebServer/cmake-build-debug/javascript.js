function getBrowser() {
    var Sys = {};  
    var ua = navigator.userAgent.toLowerCase();  
    var s;  
    (s = ua.match(/rv:([\d.]+)\) like gecko/)) ? Sys.ie = s[1] :
    (s = ua.match(/msie ([\d\.]+)/)) ? Sys.ie = s[1] :  
    (s = ua.match(/edge\/([\d\.]+)/)) ? Sys.edge = s[1] :
    (s = ua.match(/firefox\/([\d\.]+)/)) ? Sys.firefox = s[1] :  
    (s = ua.match(/(?:opera|opr).([\d\.]+)/)) ? Sys.opera = s[1] :  
    (s = ua.match(/chrome\/([\d\.]+)/)) ? Sys.chrome = s[1] :  
    (s = ua.match(/version\/([\d\.]+).*safari/)) ? Sys.safari = s[1] : 0;  
    // 根据关系进行判断
    if (Sys.ie) return ('IE');  
    if (Sys.edge) return ('Edge');
    if (Sys.firefox) return ('Firefox');  
    if (Sys.chrome) return ('Chrome');  
    if (Sys.opera) return ('Opera');  
    if (Sys.safari) return ('Safari');
    return 'Unkonwn';
}