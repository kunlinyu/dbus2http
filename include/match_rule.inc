std::string match_rule_html = R"DELIMETER====(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>

</body>
</html>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DBus Match Rule Generator</title>
</head>
<body>
<style>
    label {
        display: inline-block;
        min-width: 150px;
    }
    input {
        min-width: 300px;
    }
</style>
<div>
    <label for="type">type</label>
    <select id="type" name="type">
        <option value="">(none)</option>
        <option value="signal">signal</option>
        <option value="method_call">method_call</option>
        <option value="method_return">method_return</option>
        <option value="error">error</option>
    </select>
</div>
<div>
    <label for="sender">sender</label>
    <input type="text" id="sender" name="sender" placeholder="org.freedesktop.DBus">
</div>
<div>
    <label for="interface">interface</label>
    <input type="text" id="interface" name="interface" placeholder="org.freedesktop.DBus.Introspectable">
</div>
<div>
    <label for="member">member</label>
    <input type="text" id="member" name="member" placeholder="NameOwnerChanged">
</div>
<div>
    <label for="path">path</label>
    <input type="text" id="path" name="path" placeholder="/org/freedesktop/DBus">
</div>
<div>
    <label for="pathNamespace">namespace</label>
    <input type="text" id="pathNamespace" name="pathNamespace" placeholder="/org/freedesktop">
</div>
<div>
    <label for="destination">destination</label>
    <input type="text" id="destination" name="destination" placeholder=":1.23">
</div>
<div>
    <label for="arg0namespace">arg0 namespace</label>
    <input type="text" id="arg0namespace" name="arg0namespace" placeholder="org.freedesktop">
</div>
<div>
    <label for="eavesdrop">eavesdrop</label>
    <select id="eavesdrop" name="eavesdrop">
        <option value="">(none)</option>
        <option value="true">true</option>
        <option value="false">false</option>
    </select>
</div>
<br>
<div style="display: flex; align-items: center; gap: 10px;">
    <div id="matchRule">No parameters selected yet...</div>
    <button id="copyBtn">Copy to Clipboard</button>
    <div id="successMsg" style="display: none;">copied</div>
</div>
<br>
<div>
    <button id="listenBtn">listen</button>
    <button id="clearBtn">clear</button>
</div>
<div>
    <textarea id="messageLog" rows="10" cols="80" readonly style="margin-top: 10px; width: 1000px; height: 300px"></textarea>
</div>

<script>
    // Get elements
    const formElements = {
        type: document.getElementById('type'),
        sender: document.getElementById('sender'),
        interface: document.getElementById('interface'),
        member: document.getElementById('member'),
        path: document.getElementById('path'),
        pathNamespace: document.getElementById('pathNamespace'),
        destination: document.getElementById('destination'),
        arg0namespace: document.getElementById('arg0namespace'),
        eavesdrop: document.getElementById('eavesdrop')
    };

    const matchRuleElement = document.getElementById('matchRule');
    const copyBtn = document.getElementById('copyBtn');
    const successMsg = document.getElementById('successMsg');

    // Generate rule
    function generateMatchRule() {
        const ruleParts = [];
        if (formElements.type.value) ruleParts.push(`type='${formElements.type.value}'`);
        if (formElements.sender.value) ruleParts.push(`sender='${formElements.sender.value}'`);
        if (formElements.interface.value) ruleParts.push(`interface='${formElements.interface.value}'`);
        if (formElements.member.value) ruleParts.push(`member='${formElements.member.value}'`);
        if (formElements.path.value) ruleParts.push(`path='${formElements.path.value}'`);
        if (formElements.pathNamespace.value) ruleParts.push(`path_namespace='${formElements.pathNamespace.value}'`);
        if (formElements.destination.value) ruleParts.push(`destination='${formElements.destination.value}'`);
        if (formElements.arg0namespace.value) ruleParts.push(`arg0namespace='${formElements.arg0namespace.value}'`);
        if (formElements.eavesdrop.value) ruleParts.push(`eavesdrop='${formElements.eavesdrop.value}'`);

        matchRuleElement.textContent = ruleParts.length ? ruleParts.join(', ') : 'No parameters selected yet...';
    }

    // Copy to clipboard
    function copyToClipboard() {
        const text = matchRuleElement.textContent;
        if (text === 'No parameters selected yet...') return;

        navigator.clipboard.writeText(text).then(() => {
            successMsg.style.display = 'block';
            setTimeout(() => successMsg.style.display = 'none', 2000);
        }).catch(() => alert('Copy failed!'));
    }

    // Add listeners
    Object.values(formElements).forEach(el => {
        el.addEventListener('input', generateMatchRule);
        el.addEventListener('change', generateMatchRule);
    });
    copyBtn.addEventListener('click', copyToClipboard);

    // Initial run
    generateMatchRule();

    // Get elements
    const listenBtn = document.getElementById('listenBtn');
    const clearBtn = document.getElementById('clearBtn');
    const messageLog = document.getElementById('messageLog');
    let ws;

    // WebSocket toggle function
    function toggleWebSocket() {
        // Close existing connection if open
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.close();
            listenBtn.textContent = 'Listen';
            logMessage('Disconnected');
            return;
        }

        // Get generated match rule (encode for URL)
        const matchRule = encodeURIComponent(matchRuleElement.textContent);
        if (matchRule === encodeURIComponent('No parameters selected yet...')) {
            alert('Please generate a match rule first!');
            return;
        }

        // Create WebSocket with dynamic URL
        const wsUrl = `ws://localhost:9002/ws?match=${matchRule}`; // Replace server_address with actual host
        ws = new WebSocket(wsUrl);

        // WebSocket event handlers
        ws.onopen = () => {
            listenBtn.textContent = 'stop';
            logMessage(`Connected to: ${wsUrl}`);
        };

        ws.onmessage = (event) => {
            logMessage(`Received: ${event.data}`);
        };

        ws.onerror = (error) => {
            logMessage(`Error: ${error.message}`);
        };

        ws.onclose = (event) => {
            listenBtn.textContent = 'listen';
            logMessage(`Disconnected (Code: ${event.code}, Reason: ${event.reason})`);
        };
    }

    // Log message helper
    function logMessage(text) {
        const timestamp = new Date().toLocaleTimeString();
        messageLog.value += `[${timestamp}] ${text}\n`;
        messageLog.scrollTop = messageLog.scrollHeight; // Auto-scroll to bottom
    }

    // Add click listener
    listenBtn.addEventListener('click', toggleWebSocket);
    clearBtn.addEventListener('click', () => {messageLog.value = '';});

    document.addEventListener('DOMContentLoaded', () => {
        // Get query params from URL
        const queryParams = new URLSearchParams(window.location.search);

        // List of form fields to populate (matches element IDs and query param names)
        const formFields = [
            'type', 'sender', 'interface', 'member',
            'path', 'pathNamespace', 'destination',
            'arg0namespace', 'eavesdrop'
        ];

        // Fill each field if the query param exists
        formFields.forEach(field => {
            const value = queryParams.get(field);
            if (value) {
                document.getElementById(field).value = value;
            }
        });

        // Regenerate match rule after populating fields
        generateMatchRule();
    });
</script>
</body>
</html>

)DELIMETER====";