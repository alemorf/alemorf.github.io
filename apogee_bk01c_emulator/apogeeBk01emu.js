function apogeeBk01Emu()
{
    // Динамичкская подгрузка
    window.include = function(file)
    {
        var script = document.createElement('script');
        // script.onload = function() { ... }
        script.src = file;
        document.getElementsByTagName('head')[0].appendChild(script);
    }

    function loadAs(result)
    {
        var link = document.createElement('div');
        link.innerHTML = '<input type="file">';
        var load = link.childNodes[0];
        document.body.appendChild(link);
        load.click();
        document.body.removeChild(link);
        load.onchange = function()
        {
            var fr = new FileReader;
            fr.readAsBinaryString(load.files[0]);
            fr.onload = function(event) {
                result(load.files[0].name, event.target.result);
            };
        }
    }


    // Меню загрузки
    var loadmenu = document.getElementById('loadmenu');
    var loadbutton = document.getElementById('loadButton');
    var loadmenu_visible = false;
    
    function showLoadMenu(visible) {
        loadmenu_visible = visible;
        loadmenu.style.display = loadmenu_visible ? "block" : "none";
    }

    loadbutton.onclick = function()
    {
        showLoadMenu(!loadmenu_visible);
    }

    function loadMenuClick(name)
    {
        showLoadMenu(false);
        include("file_" + this.innerHTML + ".js");
    }

    var html = "Выберите файл для загрузки<br><br>";
    for(var i in fileList)
        html += "<div>" + fileList[i] + "</div>";
    loadmenu.innerHTML = html;

    for(var i in loadmenu.childNodes)
        loadmenu.childNodes[i].onclick = loadMenuClick;

    // Для отрисовки
    var font     = document.getElementById("apogeefont");
    var canvas   = document.getElementById("apogeecanvas");

    // Все устройства компьютера
    var memory   = new Uint8Array(65536);
    var keyboard = new Rk86Keyboard();
    var vg75     = new Vg75(canvas, font, memory);
    var vi53     = new Vi53(1777777);
    var cpu      = new I8080();

    function reset()
    {
        showLoadMenu(false);    
        for(var i in memory)
            memory[i] = 0;
        cpu.iff = 0;
        cpu.jump(0xF800);
    }

    document.getElementById('resetButton').onclick = reset;

    function binaryToArray(b)
    {
        var a = [];
        for (var i in b)
            a[i] = b.charCodeAt(i);
        return a;
    }

    function loadFile(file)
    {
        showLoadMenu(false);    
        var off = 0;
        if (file[0] == 0xE6) off++;
        var start = (file[off] << 8) | file[off + 1]; off += 2;
        var end   = (file[off] << 8) | file[off + 1]; off += 2;
        if(end < start) {
            alert("Некорректный файл");
            return;
        }
        var size = end - start + 1;
        reset();
        for(var i=0; i<3000000;)
            i += cpu.instruction();
        for (var i = 0; i < size; ++i)
            memory[start + i] = file[off + i];
        cpu.jump(start);
    }

    window.loadFile = loadFile;

    document.getElementById('loadUserButton').onclick = function() {
        showLoadMenu(false);    
        loadAs(function(fileName, data) 
        {
            loadFile(binaryToArray(data));
        });
    }

    cpu.readMemory = function(addr)
    {
        if ((addr & 0xFF03) == 0xED01) return keyboard.read(memory[0xED00]);
        if ((addr & 0xFF03) == 0xED02) return keyboard.readShifts();
        if (addr >= 0xEC00 & addr <= 0xECFF) return vi53.read(addr & 3);
        if (addr >= 0xEF00 & addr <= 0xEFFF) return vg75.read75(addr & 1);
        if (addr >= 0xF000) return apogee_rom[addr - 0xF000];
        return memory[addr];
    }

    cpu.writeMemory = function(addr, byte)
    {
        if (addr >= 0xEC00 & addr <= 0xECFF) return vi53.write(addr & 3, byte);
        if (addr >= 0xEF00 & addr <= 0xEFFF) return vg75.write75(addr & 1, byte);
        if (addr >= 0xF000 & addr <= 0xF0FF) return vg75.write57(addr & 0xFF, byte)
        if (addr < 0xF000) { memory[addr] = byte; return; }
    }

    var last_time = 0;
    const cpuSpeed = 1777777 / 1000;

    function cpuTick()
    {
        var now = new Date().getTime();
        var delta = now - last_time;
        if(delta > 500) delta = 500;
        last_time = now;

        for (var i = 0, is = delta * cpuSpeed; i < is;)
        {
            window._time = i / cpuSpeed / 1000;
            i += cpu.instruction();
        }
    }

    window.setInterval(cpuTick, 10);

    function videoTick() 
    {
        vg75.makeApogee(cpu.iff ? 128 : 0); 
    }

    window.setInterval(videoTick, 1000/60);
    reset();

    var fileInUrl = (document.URL+"").split("?");
    if(fileInUrl.length == 2) include("file_" + fileInUrl[1] + ".js");
}
