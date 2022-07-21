(function() {

// Set icon
let link = document.createElement('link');
link.rel = 'icon';
link.href = 'https://alemorf.github.io/favicon.svg';
link.type = 'image/svg+xml';
document.getElementsByTagName('head')[0].appendChild(link);

// Title
document._title_start = "Aleksey Morozov ‣ ";

// Arguments
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
window._en = (window._args["language"] == "english");

function MakeLanguageSwitchUrl() {
    let url = document.URL;

    /* Remove "language=english" from URL */
    let args_pos = url.indexOf("?");
    if (args_pos >= 0) {
        let lang_pos = args_pos + 1;
        for (;;) {
            const text = "language=english";
            lang_pos = url.indexOf(text, lang_pos);
            if (lang_pos < 0) {
                break;
            }
            const start = url.substr(lang_pos - 1, 1);
            if ((start == "?") || (start == "&")) {
                if (url.length == lang_pos + text.length) {
                    url = url.substr(0, lang_pos - 1);
                    break;
                }
                if (url.substr(lang_pos + text.length, 1) == "&") {
                    url = url.substr(0, lang_pos) + url.substr(lang_pos + text.length + 1);
                    break;
                }
            }
            lang_pos++;
        }
    }

    /* Add "language=english" */
    if (!window._en) {
        url += (args_pos >= 0 ? "&" : "?") + "language=english";
    }

    return url;
}

window.addEventListener('load', function() {
    let language_switch = document.getElementById("language_switch");
    if (language_switch) {
        language_switch.href = MakeLanguageSwitchUrl();
    }
    if (window._en) {
        let list = document.querySelectorAll("a");
        for (let i = 0; i < list.length; i++) {
            if (list[i] != language_switch) {
                const url = list[i].href;
                if ((url != "") && (url.substr(0, 8) != "https://") && (url.substr(0, 7) != "http://") && (url.substr(0, 11) != "javascript:")) {
                    list[i].href = url + (url.indexOf("?") >= 0 ? "&" : "?") + "language=english";
                }
            }
        }
    }
});

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

    document.write('<div class="h1">' + title + '<a style="float:right" id="language_switch">' + (window._en ? 'Russian' : 'English') + '</a>' + hack + '</div>');
}
