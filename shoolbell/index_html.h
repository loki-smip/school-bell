const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>School Bell Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background-color: #121212;
            color: white;
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 400px;
            margin: auto;
            background: #1E1E1E;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(255,255,255,0.1);
        }
        h2 {
            color: #FFD700;
        }
        .time-picker {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin: 10px 0;
        }
        select, button {
            padding: 10px;
            font-size: 16px;
            border-radius: 5px;
            border: none;
            cursor: pointer;
        }
        select {
            background: #333;
            color: white;
        }
        button {
            background: #007BFF;
            color: white;
        }
        button:hover {
            background: #0056b3;
        }
        .schedule-list {
            list-style: none;
            padding: 0;
        }
        .schedule-item {
            background: #222;
            margin: 5px 0;
            padding: 10px;
            border-radius: 5px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .delete-btn {
            background: #FF5733;
            padding: 5px 10px;
            font-size: 14px;
        }
        .delete-btn:hover {
            background: #C70039;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>🔔 School Bell Scheduler</h2>
        
        <h3>Set Bell Time</h3>
        <div class="time-picker">
            <select id="hour"></select> :
            <select id="minute"></select>
            <select id="ampm">
                <option value="false">AM</option>
                <option value="true">PM</option>
            </select>
        </div>
        <button onclick="setBellTime()">➕ Add Schedule</button>
        
        <h3>Scheduled Times</h3>
        <ul id="scheduleList" class="schedule-list"></ul>
        
        <button onclick="ringBell()">🔔 Ring Bell Now</button>
    </div>
    
    <script>
        // Populate hour & minute options
        let hourSelect = document.getElementById("hour");
        let minuteSelect = document.getElementById("minute");
        for (let i = 1; i <= 12; i++) {
            let opt = document.createElement("option");
            opt.value = i;
            opt.textContent = i;
            hourSelect.appendChild(opt);
        }
        for (let i = 0; i < 60; i++) {
            let opt = document.createElement("option");
            opt.value = i;
            opt.textContent = i.toString().padStart(2, '0');
            minuteSelect.appendChild(opt);
        }

        function setBellTime() {
            let hour = hourSelect.value;
            let minute = minuteSelect.value;
            let isPM = document.getElementById("ampm").value;
            fetch(`/set?hour=${hour}&minute=${minute}&isPM=${isPM}`, { method: 'POST' })
                .then(() => location.reload());
        }

        function ringBell() {
            fetch('/ring');
        }

        function loadSchedule() {
            fetch('/schedules')
                .then(response => response.json())
                .then(data => {
                    let list = document.getElementById("scheduleList");
                    list.innerHTML = "";
                    data.forEach((item, index) => {
                        let li = document.createElement("li");
                        li.className = "schedule-item";
                        li.innerHTML = `
                            ${item.hour}:${item.minute.toString().padStart(2, '0')} ${item.isPM ? "PM" : "AM"}
                            <button class="delete-btn" onclick="deleteSchedule(${index})">❌</button>
                        `;
                        list.appendChild(li);
                    });
                });
        }

        function deleteSchedule(index) {
            fetch(`/delete?index=${index}`, { method: 'POST' })
                .then(() => location.reload());
        }

        loadSchedule();
    </script>
</body>
</html>
)rawliteral";
