const app = {
    init: async () => {
        // Fetch project status
        try {
            const res = await fetch('/api/project/status');
            if (res.ok) {
                const status = await res.json();
                document.getElementById('project-path').innerText = status.path;
            }
        } catch (e) {
            console.error("Failed to connect to backend");
        }
    },

    buildProject: async () => {
        app.log("Starting build...");
        try {
            // Use the existing build API we created for theme editor, 
            // but maybe generalize it? Or just call it.
            // Tools.cpp exposes /api/system/build which builds the CURRENT project root.
            const res = await fetch('/api/system/build', { method: 'POST' });
            if (res.ok) {
                app.log("Build Successful!");
                alert("Site built successfully!");
            } else {
                app.log("Build Failed. Check server logs.");
                alert("Build Failed");
            }
        } catch (e) {
            app.log("Error: " + e.message);
        }
    },

    browseProject: () => {
        // Placeholder for now, requires a file browser UI or prompt
        const newPath = prompt("Enter new project path (absolute):");
        if (newPath) {
            fetch('/api/project/set-root', {
                method: 'POST',
                body: newPath
            }).then(() => window.location.reload());
        }
    },

    exitSystem: async () => {
        if (confirm("Are you sure you want to stop the server and exit?")) {
            await fetch('/api/system/exit', { method: 'POST' });
            document.body.innerHTML = "<h1 style='color:white;text-align:center;margin-top:20%'>Server Stopped. You can close this tab.</h1>";
        }
    },

    log: (msg) => {
        const consoleEl = document.getElementById('log-output');
        const time = new Date().toLocaleTimeString();
        consoleEl.textContent += `[${time}] ${msg}\n`;
        consoleEl.scrollTop = consoleEl.scrollHeight;
    },

    clearLogs: () => {
        document.getElementById('log-output').textContent = "";
    }
};

// Make globally available for onclick handlers
window.app = app;
document.addEventListener('DOMContentLoaded', app.init);
