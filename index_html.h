#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1" />
    <title>Key 32 Studio</title>

    <style>
    :root {
        --bg: #0a0c10;
        --surface: #12161e;
        --surface2: #1a2030;
        --border: #2a3448;
        --accent: #00d4ff;
        --accent2: #ff6b35;
        --green: #39ff14;
        --text: #c8d8f0;
        --muted: #4a5a78;
        --danger: #ff2d55;
    }

    * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
    }

    body {
        background: var(--bg);
        color: var(--text);
        font-family: 'Exo 2', sans-serif;
        min-height: 100vh;
    }

    header {
        display: flex;
        align-items: center;
        gap: 16px;
        padding: 20px 32px;
        border-bottom: 1px solid var(--border);
    }

    main {
        max-width: 900px;
        margin: 0 auto;
        padding: 32px 24px;
        display: flex;
        flex-direction: column;
        gap: 32px;
    }

    section {
        background: var(--surface);
        border: 1px solid var(--border);
        border-radius: 4px;
    }

    .sec-header {
        display: flex;
        align-items: center;
        gap: 12px;
        padding: 14px 24px;
        background: var(--surface2);
    }

    .sec-body {
        padding: 24px;
    }

    .grid2 {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 16px;
    }

    .field {
        display: flex;
        flex-direction: column;
        gap: 6px;
    }

    input {
        background: var(--bg);
        border: 1px solid var(--border);
        color: var(--text);
        padding: 10px;
    }

    table {
        width: 100%;
    }

    td {
        text-align: center;
        padding: 5px;
    }

    .test_btn {
        background: #fff9c0;
        border: 2px solid #f5e27a;
        border-radius: 8px;
        width: 50px;
        height: 50px;
        font-size: 16px;
        font-weight: bold;
        color: #333;
        box-shadow: inset 0 2px 4px rgba(0,0,0,0.2), 0 4px 6px rgba(0,0,0,0.15);
        cursor: pointer;
        display: inline-flex;
        align-items: center;
        justify-content: center;
    }

    .test_btn:hover {
        background: #fffec0;
        box-shadow: inset 0 3px 6px rgba(0,0,0,0.2), 0 4px 6px rgba(255, 255, 255, 0.38);
    }

    .btns_div {
        display: flex;
        justify-content: center;
        align-items: center;
    }

    .btn {
        border-radius: 8px;
        padding: 10px 16px;
        cursor: pointer;
        font-weight: bold;
        margin: 5px;
    }

    .btn-primary {
        background: var(--accent);
        color: #000;
    }

    .btn-ghost {
        background: transparent;
        color: #fff;
    }

    .btn-danger {
        color: red;
    }
    </style>
  </head>

  <body>
    <header>
      <h1>Key 32 Studio</h1>
    </header>

    <main>
      <section>
        <div class="sec-header">WiFi</div>
        <div class="sec-body grid2">
          <div class="field">
            <label>SSID</label>
            <input type="text" id="wifi_ssid" />
          </div>

          <div class="field">
            <label>Mot de passe</label>
            <input type="password" id="wifi_password" />
          </div>
        </div>
      </section>

      <section>
        <div class="sec-header">Bitfocus Companion</div>
        <div class="sec-body grid2">
          <div class="field">
            <label>Adresse IP</label>
            <input type="text" id="companion_ip" />
          </div>

          <div class="field">
            <label>Port HTTP</label>
            <input type="number" id="companion_port" value="8888" />
          </div>
        </div>
      </section>

      <section>
        <div class="sec-header">Touches</div>
        <div class="sec-body">
          <div id="keys-container"></div>
        </div>
      </section>

      <div class="btns_div">
        <button class="btn btn-primary" onclick="saveConfig()">Sauvegarder</button>
        <button class="btn btn-ghost" onclick="loadConfig()">Recharger</button>
        <button class="btn btn-danger" onclick="reboot()">Redémarrer</button>
      </div>
    </main>

    <script>
        var cfg={keys:[],companion_ip:'',companion_port:8888};

        function buildUrl(i) {
            var ip =
            document.getElementById('companion_ip').value ||
            cfg.companion_ip ||
            '...';

            var port =
            document.getElementById('companion_port').value ||
            cfg.companion_port ||
            8888;

            var pg = document.getElementById('k' + i + '_page')
            ? document.getElementById('k' + i + '_page').value
            : (cfg.keys[i] ? cfg.keys[i].page : 1);

            var rw = document.getElementById('k' + i + '_row')
            ? document.getElementById('k' + i + '_row').value
            : (cfg.keys[i] ? cfg.keys[i].row : 0);

            var cl = document.getElementById('k' + i + '_col')
            ? document.getElementById('k' + i + '_col').value
            : (cfg.keys[i] ? cfg.keys[i].col : 0);

            return 'http://'+ip+':'+port+'/api/location/'+pg+'/'+rw+'/'+cl+'/<b>down</b> | <b>up</b>';
        }

        function buildKeyCards(data){
        cfg = data;

        var c = document.getElementById('keys-container');
        c.innerHTML = '';

        var h = "";

        h += "<table><thead><tr>";
        h += "<th>#</th><th>Label</th><th>GPIO</th><th>Page</th><th>Row</th><th>Col</th><th>On</th><th>Test</th>";
        h += "</tr></thead><tbody>";

        data.keys.forEach(function(k,i){
            h += "<tr>";

            h += "<td>"+(i+1)+"</td>";

            h += "<td><input type='text' id='k"+i+"_label' value='"+k.label+"'></td>";

            h += "<td>"+k.pin+"</td>";

            h += "<td><input type='number' id='k"+i+"_page' value='"+k.page+"' min='1' max='99'></td>";

            h += "<td><input type='number' id='k"+i+"_row' value='"+k.row+"' min='0' max='7'></td>";

            h += "<td><input type='number' id='k"+i+"_col' value='"+k.col+"' min='0' max='7'></td>";

            h += "<td><input type='checkbox' id='k"+i+"_enabled' "+(k.enabled?"checked":"")+"></td>";

            h += "<td><button class='test_btn' onclick='testKey("+i+")'>Test</button></td>";

            h += "</tr>";
        });

        h += "</tbody></table>";

        c.innerHTML = h;
        }



        function loadConfig() {
            fetch('/api/config')
            .then(function (r) {
                return r.json();
            })
            .then(function (d) {
                document.getElementById('wifi_ssid').value = d.wifi_ssid || '';
                document.getElementById('wifi_password').value = '';

                document.getElementById('companion_ip').value =
                d.companion_ip || '';
                document.getElementById('companion_port').value =
                d.companion_port || 8888;

                buildKeyCards(d);

                showToast('Config chargee', true);
            })
            .catch(function () {
                showToast('Erreur chargement', false);
            });
        }

        function saveConfig() {
            var keys = cfg.keys.map(function (_, i) {
            return {
                label: document.getElementById('k' + i + '_label').value,
                page: +document.getElementById('k' + i + '_page').value,
                row: +document.getElementById('k' + i + '_row').value,
                col: +document.getElementById('k' + i + '_col').value,
                enabled: document.getElementById('k' + i + '_enabled').checked
            };
            });

            var payload = {
            wifi_ssid: document.getElementById('wifi_ssid').value,
            wifi_password: document.getElementById('wifi_password').value,
            companion_ip: document.getElementById('companion_ip').value,
            companion_port: +document.getElementById('companion_port').value,
            keys: keys
            };

            fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
            })
            .then(function (r) {
                return r.json();
            })
            .then(function (d) {
                showToast(
                d.status === 'ok' ? 'Sauvegarde OK' : 'Erreur serveur',
                d.status === 'ok'
                );
            })
            .catch(function () {
                showToast('Erreur reseau', false);
            });
        }

        function testKey(idx) {
            fetch('/api/test', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ key: idx })
            })
            .then(function () {
                showToast('Key ' + (idx + 1) + ' - press envoye', true);
            })
            .catch(function () {
                showToast('Erreur test', false);
            });
        }

        function reboot() {
            if (!confirm('Redemarrer l ESP32 ?')) {
            return;
            }

            fetch('/api/reboot', { method: 'POST' });

            showToast('Redemarrage...', true);
        }

        function showToast(msg, ok) {
            var t = document.getElementById('toast');

            t.textContent = msg;
            t.className = 'toast ' + (ok ? 'ok' : 'err');
            t.style.display = 'block';

            clearTimeout(t._t);
            t._t = setTimeout(function () {
            t.style.display = 'none';
            }, 3000);
        }

        loadConfig();
    </script>

  </body>
</html>
)rawliteral";