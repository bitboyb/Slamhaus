const editorApp = {
    setStatus: (message) => {
        const status = document.getElementById('system-status');
        if (status) {
            status.textContent = message;
        }
    },

    init: async () => {
        const projectPath = document.getElementById('project-path');
        if (!projectPath) {
            return;
        }

        editorApp.setStatus("Connecting...");

        try {
            const res = await fetch('/api/project/status');
            if (res.ok) {
                const status = await res.json();
                projectPath.textContent = status.path;
                editorApp.setStatus("Ready");
            } else {
                editorApp.setStatus("Unavailable");
            }
        } catch (e) {
            console.error("Failed to connect to backend", e);
            projectPath.textContent = "Backend unavailable.";
            editorApp.setStatus("Offline");
        }
    },

    buildProject: async () => {
        editorApp.log("Starting build...");
        editorApp.setStatus("Building...");
        try {
            const res = await fetch('/api/project/build', { method: 'POST' });
            if (res.ok) {
                editorApp.log("Build successful.");
                editorApp.setStatus("Built");
            } else {
                editorApp.log("Build failed. Check server logs.");
                editorApp.setStatus("Build failed");
            }
        } catch (e) {
            editorApp.log("Error: " + e.message);
            editorApp.setStatus("Build failed");
        }
    },

    browseProject: () => {
        const newPath = prompt("Enter new project path (absolute):");
        if (newPath) {
            editorApp.setStatus("Switching project...");
            fetch('/api/project/set-root', {
                method: 'POST',
                body: newPath
            }).then(() => window.location.reload());
        }
    },

    exitEditor: async () => {
        if (!confirm("Are you sure you want to stop the server and exit?")) {
            return;
        }

        editorApp.setStatus("Stopping...");
        await fetch('/api/editor/exit', { method: 'POST' });
        document.body.textContent = "Editor stopped. Press Enter in the terminal to finish shutting it down.";
    },

    log: (msg) => {
        const consoleEl = document.getElementById('log-output');
        if (!consoleEl) {
            return;
        }

        const time = new Date().toLocaleTimeString();
        consoleEl.textContent += `[${time}] ${msg}\n`;
        consoleEl.scrollTop = consoleEl.scrollHeight;
    },

    clearLogs: () => {
        const consoleEl = document.getElementById('log-output');
        if (consoleEl) {
            consoleEl.textContent = "";
        }
    }
};

window.editorApp = editorApp;
document.addEventListener('DOMContentLoaded', () => {
    editorApp.init();
});
