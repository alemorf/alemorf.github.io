(function() {

document._title_start = "Aleksey Morozov ‣ ";

const url = document.URL;
let queryString = url ? url.split('?')[1] : window.location.search.slice(1);
let obj = {};
if (queryString) {
    queryString = queryString.split('#')[0];
    let arr = queryString.split('&');
    for (let i = 0; i < arr.length; i++) {
        let a = arr[i].split('=');
        let paramNum = undefined;
        let paramName = a[0].replace(/\[\d*\]/, function(v) {
            paramNum = v.slice(1, -1);
            return '';
        });
        let paramValue = typeof (a[1]) === 'undefined' ? true : a[1];
        paramName = paramName;
        paramValue = decodeURIComponent(paramValue);
        if (obj[paramName]) {
            if (typeof obj[paramName] === 'string') {
                obj[paramName] = [ obj[paramName] ];
            }
            if (typeof paramNum === 'undefined') {
                obj[paramName].push(paramValue);
            } else {
                obj[paramName][paramNum] = paramValue;
            }
        } else {
            obj[paramName] = paramValue;
        }
    }
}

window._args = obj;

})();

function StartPage(title) {
    document.title = document._title_start + title;
    
    function DuplicateString(string, deep) {
        if (deep < 1) {
            return "";
        }
        if (deep < 2) {
            return string;
        }
        const left = Math.trunc(deep / 2);
        return DuplicateString(string, left) +
               DuplicateString(string, deep - left);
    }

    document.write('<div id="main"><div>');

    const main_full_width_px = 16;
    const main_width_px = 1280;
    const hack = DuplicateString('<span class="main_full"></span>',
                                 main_width_px / main_full_width_px);
    if (title != "") {
        document.write('<div class="h1">' + title + hack + '</div>');
    } else {
        document.write(hack);
    }
}
