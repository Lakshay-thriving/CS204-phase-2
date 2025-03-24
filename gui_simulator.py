from flask import Flask, request, render_template_string
import subprocess
import re

app = Flask(__name__)

HTML = """
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>RISC-V Simulator</title>
  <!-- Import Orbitron font for the title -->
  <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@500;700&display=swap" rel="stylesheet">
  <style>
    /* Global Styles */
    body {
      font-family: 'Orbitron', sans-serif;
      margin: 0;
      padding: 0;
      background: linear-gradient(135deg, #000, #333);
      color: #fff;
      overflow-x: hidden;
    }
    
    .container {
      max-width: 1200px;
      margin: 50px auto;
      background: rgba(20, 20, 20, 0.95);
      padding: 40px;
      border-radius: 10px;
      box-shadow: 0 12px 30px rgba(0, 0, 0, 0.8);
      position: relative;
      overflow: hidden;
    }
    
    /* Background Animation */
    .container::before {
      content: "";
      position: absolute;
      top: -50%;
      left: -50%;
      width: 200%;
      height: 200%;
      background: radial-gradient(circle at center, rgba(255,215,0,0.15), transparent 70%);
      animation: pulse 6s infinite;
      z-index: 0;
    }
    
    @keyframes pulse {
      0%, 100% { transform: scale(1); opacity: 0.8; }
      50% { transform: scale(1.2); opacity: 1; }
    }
    
    .container > * {
      position: relative;
      z-index: 1;
    }
    
    h1, h2 {
      text-align: center;
      color: #ffd700; /* Golden Yellow */
      margin-bottom: 20px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.7);
    }
    
    h1 {
      font-size: 2.5em;
    }
    
    textarea {
      width: 100%;
      height: 220px;
      font-family: 'Courier New', monospace;
      padding: 15px;
      border: 2px solid #ffd700;
      border-radius: 6px;
      margin-bottom: 20px;
      background: #222;
      color: #fff;
      font-size: 14px;
      resize: vertical;
      transition: border-color 0.3s;
    }
    
    textarea:focus {
      outline: none;
      border-color: #ffc107;
    }
    
    .center-button {
      text-align: center;
      margin-bottom: 30px;
    }
    
    input[type="submit"] {
      background: #ffd700;
      color: #000;
      border: none;
      padding: 15px 30px;
      font-size: 16px;
      font-weight: bold;
      border-radius: 6px;
      cursor: pointer;
      display: block;
      margin: 0 auto 30px auto;
      box-shadow: 0 4px 10px rgba(0,0,0,0.5);
      transition: background 0.3s, transform 0.3s;
    }
    
    input[type="submit"]:hover {
      background: #e6c200;
      transform: scale(1.05);
    }
    
    pre {
      background: #111;
      padding: 20px;
      border: 1px solid #444;
      border-radius: 6px;
      font-family: 'Courier New', monospace;
      white-space: pre-wrap;
      overflow-x: auto;
    }
    
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 20px;
      font-size: 14px;
    }
    
    table th, table td {
      border: 1px solid #444;
      padding: 10px;
      text-align: left;
    }
    
    table th {
      background: #ffd700;
      color: #000;
    }
    
    .section {
      margin-bottom: 40px;
    }
    
    footer {
      margin-top: 40px;
      text-align: center;
      font-size: 14px;
      color: #ffd700;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>RISC-V Simulator</h1>
    <form method="post">
      <div class="section">
        <h2>Input Assembly Code</h2>
        <textarea name="assembly_code">{{ assembly_code }}</textarea>
        <div class="center-button">
          <input type="submit" value="Run Simulation">
        </div>
      </div>
    </form>
    {% if sim_output %}
    <div class="section">
      <h2>Simulation Output</h2>
      <pre>{{ sim_output }}</pre>
    </div>
    {% endif %}
    {% if reg_file %}
    <div class="section">
      <h2>Register File</h2>
      <table>
        <tr>
          <th>Register</th>
          <th>Value</th>
        </tr>
        {% for reg, value in reg_file.items() %}
        <tr>
          <td>{{ reg }}</td>
          <td>{{ value }}</td>
        </tr>
        {% endfor %}
      </table>
    </div>
    {% endif %}
    {% if data_memory %}
    <div class="section">
      <h2>Data Memory (from 0x10000000)</h2>
      <table>
        <tr>
          <th>Address</th>
          <th>Hex Value</th>
          <th>Label/Info</th>
        </tr>
        {% for entry in data_memory %}
        <tr>
          <td>{{ entry.address }}</td>
          <td>{{ entry.hex_value }}</td>
          <td>{{ entry.info }}</td>
        </tr>
        {% endfor %}
      </table>
    </div>
    {% endif %}
    {% if stack_memory %}
    <div class="section">
      <h2>Stack Memory</h2>
      <table>
        <tr>
          <th>Address</th>
          <th>Value</th>
        </tr>
        {% for entry in stack_memory %}
        <tr>
          <td>{{ entry.address }}</td>
          <td>{{ entry.value }}</td>
        </tr>
        {% endfor %}
      </table>
    </div>
    {% endif %}
    <footer>
      Developed by: Lakshay Kumar, Harsh Asodariya, Vivan Garg
    </footer>
  </div>
</body>
</html>
"""

def get_data_memory(mc_text):
    """
    Parses the machine code output (mc_text) to extract data memory entries.
    Only lines in the data segment starting with an address (e.g. 0x10000000 or higher)
    are included.
    """
    data_memory = []
    in_data_segment = False
    for line in mc_text.splitlines():
        line = line.strip()
        if line.startswith("# --- DATA SEGMENT ---"):
            in_data_segment = True
            continue
        if line.startswith("# ---") and in_data_segment:
            in_data_segment = False
        if in_data_segment and line.startswith("0x"):
            parts = line.split(",", 1)
            if len(parts) < 2:
                continue
            tokens = parts[0].split()
            if len(tokens) < 2:
                continue
            address = tokens[0]
            hex_value = tokens[1]
            info = parts[1].strip()
            try:
                if int(address, 16) >= 0x10000000:
                    data_memory.append({
                        "address": address,
                        "hex_value": hex_value,
                        "info": info
                    })
            except Exception:
                continue
    return data_memory

def get_register_file(sim_console_output):
    """
    Builds a dummy register file for demonstration.
    Replace or extend this function to reflect actual register states from your simulator.
    """
    reg_file = {f"x{i}": 0 for i in range(32)}
    reg_file["x0"] = 0
    reg_file["x1"] = 5
    return reg_file

def get_stack_memory():
    """
    Generates a dummy stack memory table.
    In this example, we assume the stack starts at 0x7FFFFFF0.
    """
    stack_memory = []
    start_address = 0x7FFFFFF0
    for i in range(8):
        address = hex(start_address - i * 4)
        stack_memory.append({"address": address, "value": 0})
    return stack_memory

@app.route("/", methods=["GET", "POST"])
def index():
    assembly_code = ""
    sim_output = ""
    reg_file = {}
    data_memory = []
    stack_memory = []
    
    if request.method == "POST":
        assembly_code = request.form.get("assembly_code", "")
        # Normalize the code: replace Windows newlines with Unix, strip trailing spaces,
        # and remove extra blank lines.
        normalized_code = "\n".join(line.rstrip() for line in assembly_code.replace("\r\n", "\n").split("\n") if line.strip() != "")
        try:
            # Write the normalized assembly code to input.asm
            with open("input.asm", "w", encoding="utf-8") as f:
                f.write(normalized_code)
        except Exception as e:
            sim_output = "Error writing to input.asm:\n" + str(e)
            return render_template_string(HTML, assembly_code=assembly_code, sim_output=sim_output)
        
        try:
            # Run the simulator executable (code.exe) and capture both stdout and stderr.
            process = subprocess.run(["code.exe"], shell=True, capture_output=True, text=True)
            sim_console = process.stdout + process.stderr
        except Exception as e:
            sim_console = "Error running simulator:\n" + str(e)
        
        try:
            with open("output.mc", "r", encoding="utf-8") as f:
                mc_output = f.read()
        except Exception:
            mc_output = ""
        
        # Combine console output and machine code output.
        sim_output = sim_console + "\n\n" + mc_output
        
        # Parse extra information for display.
        data_memory = get_data_memory(mc_output)
        reg_file = get_register_file(sim_console)
        stack_memory = get_stack_memory()
    
    return render_template_string(HTML, 
                                  assembly_code=assembly_code, 
                                  sim_output=sim_output,
                                  reg_file=reg_file,
                                  data_memory=data_memory,
                                  stack_memory=stack_memory)

if __name__ == "__main__":
    print("Starting Flask server on http://127.0.0.1:5000/")
    app.run(debug=True)