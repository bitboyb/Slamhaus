export async function Init() {
    console.log("Initializing Theme Editor...");
    const controls = document.getElementById('controls');
    const preview = document.getElementById('preview-frame');
    const saveBtn = document.getElementById('save-btn');

    if (!controls) {
        console.error("Controls container not found!");
        return;
    }

    // Map filename -> content string
    let fileContents = {};
    // Map filename -> { key: value }
    let fileVars = {};

    // 1. Fetch File List
    let files = [];
    try {
        const res = await fetch('/api/theme/files');
        if (!res.ok) throw new Error(res.statusText);
        files = await res.json();
        console.log("CSS Files found:", files);
    } catch (e) {
        console.error("Failed to load file list", e);
        controls.innerHTML = "Error loading files.<br>" + e.message;
        return;
    }

    // 2. Fetch Content for each file
    for (const filename of files) {
        try {
            const res = await fetch('/api/theme/read', {
                headers: { 'X-File': filename }
            });
            if (!res.ok) continue;
            const content = await res.text();
            fileContents[filename] = content;

            // Parse Variables
            fileVars[filename] = {};
            const regex = /--([a-zA-Z0-9-_]+):\s*([^;]+);/g;
            let match;
            while ((match = regex.exec(content)) !== null) {
                const key = match[1];
                let value = match[2].trim();
                if (value.includes("/*")) {
                    value = value.split("/*")[0].trim();
                }
                fileVars[filename][key] = value;
            }
        } catch (e) {
            console.error("Error loading " + filename, e);
        }
    }

    // 3. Render Controls
    controls.innerHTML = '';

    // Sort files to put palette.css first if exists
    files.sort((a, b) => {
        if (a === 'palette.css') return -1;
        if (b === 'palette.css') return 1;
        return a.localeCompare(b);
    });

    let totalVars = 0;

    for (const filename of files) {
        const vars = fileVars[filename];
        if (!vars || Object.keys(vars).length === 0) continue;

        totalVars += Object.keys(vars).length;

        // Group Header
        const groupHeader = document.createElement('h3');
        groupHeader.innerText = filename;
        groupHeader.style.marginTop = '20px';
        groupHeader.style.marginBottom = '10px';
        groupHeader.style.color = '#ddd'; // Editor CSS handles this but inline for safety
        groupHeader.style.borderBottom = '1px solid #444';
        groupHeader.style.paddingBottom = '5px';
        controls.appendChild(groupHeader);

        // Grid Container
        const gridDiv = document.createElement('div');
        gridDiv.className = 'controls-grid';
        controls.appendChild(gridDiv);

        const keys = Object.keys(vars).sort();
        for (const key of keys) {
            createControl(filename, key, vars[key], gridDiv);
        }
    }

    if (totalVars === 0) {
        controls.innerHTML = "No CSS variables found in any file.";
    }

    function createControl(filename, key, value, container) {
        const div = document.createElement('div');
        div.className = 'control-group';

        // Side-by-side layout for label and input
        div.style.display = 'flex';
        div.style.justifyContent = 'space-between';
        div.style.alignItems = 'center';

        const label = document.createElement('label');
        label.innerText = key.replace(/--/g, '').replace(/-/g, ' ').toUpperCase();
        label.style.marginBottom = '0';
        label.style.flex = '1';
        label.style.marginRight = '10px';
        label.style.wordBreak = 'break-word';
        div.appendChild(label);

        const input = document.createElement('input');
        input.style.flex = '1';

        // Simple heuristic for color input
        const isColor = /^#[0-9A-F]{6}$/i.test(value) || /^#[0-9A-F]{3}$/i.test(value);

        if (isColor) {
            input.type = 'color';
        } else {
            input.type = 'text';
        }
        input.value = value;

        input.addEventListener('input', (e) => {
            const newValue = e.target.value;
            // Update local state
            fileVars[filename][key] = newValue;
            // Live Preview
            UpdatePreview(key, newValue);
        });

        div.appendChild(input);
        container.appendChild(div);
    };

    // 4. Save Handler
    if (saveBtn) {
        saveBtn.addEventListener('click', async () => {
            // 1. Confirmation
            if (!confirm("Are you sure you want to save? This will override your current theme.")) {
                return;
            }

            saveBtn.innerText = "Saving...";
            let errors = 0;

            for (const filename of files) {
                const vars = fileVars[filename];
                if (!vars || Object.keys(vars).length === 0) continue;

                // Reconstruct content
                let content = fileContents[filename];
                for (const [key, value] of Object.entries(vars)) {
                    const keyRegex = new RegExp(`(--${key}:\\s*)([^;]+)(;)`, 'g');
                    content = content.replace(keyRegex, `$1${value}$3`);
                }

                try {
                    const res = await fetch('/api/theme/write', {
                        method: 'POST',
                        headers: { 'X-File': filename },
                        body: content
                    });
                    if (!res.ok) throw new Error(`${res.status} ${res.statusText}: ${await res.text()}`);

                    // Update original map
                    fileContents[filename] = content;
                } catch (e) {
                    console.error("Failed to save " + filename, e);
                    errors++;
                    alert("Error saving " + filename + ": " + e.message);
                }
            }

            if (errors === 0) {
                // 2. Auto-Build
                saveBtn.innerText = "Building...";
                try {
                    const buildRes = await fetch('/api/system/build', { method: 'POST' });
                    if (buildRes.ok) {
                        saveBtn.innerText = "Saved & Built!";
                        setTimeout(() => saveBtn.innerText = "Save Theme", 2000);
                        alert("Build Successful!");
                    } else {
                        throw new Error("Build failed");
                    }
                } catch (e) {
                    saveBtn.innerText = "Saved (Build Failed)";
                    alert("Theme saved but site build failed.");
                }
            } else {
                saveBtn.innerText = "Error Saving";
                alert(`Failed to save ${errors} files.`);
            }
        });
    }

    // 5. Update Preview & Self Logic
    const UpdatePreview = (key, value) => {
        // A. Update Editor Itself: DISABLED (Strict Isolation)
        // document.documentElement.style.setProperty('--' + key, value);

        // B. Update Iframe
        try {
            if (preview.contentDocument && preview.contentDocument.documentElement) {
                preview.contentDocument.documentElement.style.setProperty('--' + key, value);
            }
        } catch (e) {
            console.warn("Could not inject CSS directly", e);
        }
    };

    // Initial apply to PREVIEW only
    preview.onload = () => {
        Object.keys(fileVars).forEach(fname => {
            const vars = fileVars[fname];
            for (const [key, value] of Object.entries(vars)) {
                UpdatePreview(key, value);
            }
        });
    };
}
