const themeEditor = {
    controls: null,
    fileSwitcher: null,
    preview: null,
    previewShell: null,
    previewCaption: null,
    mobilePreviewBtn: null,
    desktopPreviewBtn: null,
    saveBtn: null,
    files: [],
    fileContents: {},
    fileVars: {},
    activeFile: '',
    previewMode: 'mobile',
    previewModes: {
        mobile: {
            label: 'Generic mobile preview',
            width: 390,
            height: 844
        },
        desktop: {
            label: 'Generic desktop preview',
            width: 1440,
            height: 900
        }
    },

    init: async () => {
        themeEditor.controls = document.getElementById('controls');
        themeEditor.fileSwitcher = document.getElementById('file-switcher');
        themeEditor.preview = document.getElementById('preview-frame');
        themeEditor.previewShell = document.getElementById('preview-shell');
        themeEditor.previewCaption = document.getElementById('preview-caption');
        themeEditor.mobilePreviewBtn = document.getElementById('mobile-preview');
        themeEditor.desktopPreviewBtn = document.getElementById('desktop-preview');
        themeEditor.saveBtn = document.getElementById('save-btn');

        if (!themeEditor.controls || !themeEditor.fileSwitcher || !themeEditor.preview ||
            !themeEditor.previewShell || !themeEditor.previewCaption ||
            !themeEditor.saveBtn) {
            return;
        }

        themeEditor.controls.textContent = "Loading controls...";
        themeEditor.fileSwitcher.textContent = "Loading files...";
        themeEditor.fileContents = {};
        themeEditor.fileVars = {};
        themeEditor.files = [];
        themeEditor.setPreviewMode(themeEditor.previewMode);
        window.addEventListener('resize', themeEditor.updatePreviewScale);

        try {
            const res = await fetch('/api/theme/files');
            if (!res.ok) {
                throw new Error(res.statusText);
            }
            themeEditor.files = await res.json();
        } catch (e) {
            console.error("Failed to load file list", e);
            themeEditor.controls.textContent = "Theme controls require the Slamhaus editor server.";
            return;
        }

        for (const filename of themeEditor.files) {
            try {
                const res = await fetch('/api/theme/read', {
                    headers: { 'X-File': filename }
                });
                if (!res.ok) {
                    continue;
                }

                const content = await res.text();
                themeEditor.fileContents[filename] = content;
                themeEditor.fileVars[filename] = {};

                const regex = /--([a-zA-Z0-9-_]+):\s*([^;]+);/g;
                let match;
                while ((match = regex.exec(content)) !== null) {
                    const key = match[1];
                    let value = match[2].trim();
                    if (value.includes("/*")) {
                        value = value.split("/*")[0].trim();
                    }
                    themeEditor.fileVars[filename][key] = value;
                }
            } catch (e) {
                console.error("Error loading " + filename, e);
            }
        }

        themeEditor.files = themeEditor.files.filter((filename) => {
            const vars = themeEditor.fileVars[filename];
            return vars && Object.keys(vars).length > 0;
        });

        if (themeEditor.files.includes('spacing.css')) {
            themeEditor.activeFile = 'spacing.css';
        } else if (themeEditor.files.includes('palette.css')) {
            themeEditor.activeFile = 'palette.css';
        } else if (!themeEditor.activeFile && themeEditor.files.length > 0) {
            themeEditor.activeFile = themeEditor.files[0] || '';
        }

        themeEditor.renderControls();
        themeEditor.preview.onload = () => {
            Object.keys(themeEditor.fileVars).forEach((filename) => {
                const vars = themeEditor.fileVars[filename];
                for (const [key, value] of Object.entries(vars)) {
                    themeEditor.updatePreview(key, value);
                }
            });
            themeEditor.updatePreviewScale();
        };
    },

    setPreviewMode: (mode) => {
        const nextMode = themeEditor.previewModes[mode];
        if (!nextMode || !themeEditor.previewShell || !themeEditor.previewCaption) {
            return;
        }

        themeEditor.previewMode = mode;
        themeEditor.previewShell.dataset.preview = mode;
        themeEditor.previewShell.style.setProperty('--preview-width', `${nextMode.width}px`);
        themeEditor.previewShell.style.setProperty('--preview-height', `${nextMode.height}px`);
        themeEditor.previewCaption.textContent = nextMode.label;

        if (themeEditor.mobilePreviewBtn) {
            themeEditor.mobilePreviewBtn.setAttribute('aria-pressed', mode === 'mobile' ? 'true' : 'false');
        }

        if (themeEditor.desktopPreviewBtn) {
            themeEditor.desktopPreviewBtn.setAttribute('aria-pressed', mode === 'desktop' ? 'true' : 'false');
        }

        themeEditor.updatePreviewScale();
    },

    showMobilePreview: () => {
        themeEditor.setPreviewMode('mobile');
    },

    showDesktopPreview: () => {
        themeEditor.setPreviewMode('desktop');
    },

    updatePreviewScale: () => {
        const current = themeEditor.previewModes[themeEditor.previewMode];
        if (!current || !themeEditor.previewShell) {
            return;
        }

        const availableWidth = themeEditor.previewShell.clientWidth || themeEditor.previewShell.parentElement?.clientWidth || current.width;
        const scale = Math.min(availableWidth / current.width, 1);
        themeEditor.previewShell.style.setProperty('--preview-scale', `${scale}`);
    },

    getSectionsForFile: (filename, vars) => {
        const keys = Object.keys(vars);
        const sectionsByFile = {
            'palette.css': [
                { title: 'Backgrounds', note: 'palette.css', match: (key) => key.startsWith('color-background-') },
                { title: 'Borders', note: 'palette.css', match: (key) => key.startsWith('color-border-') },
                { title: 'Text', note: 'palette.css', match: (key) => key.startsWith('color-text-') },
                { title: 'Accents', note: 'palette.css', match: (key) => key.startsWith('color-accent-') },
                { title: 'Shadows', note: 'palette.css', match: (key) => key.startsWith('color-shadow-') || key === 'color-19' }
            ],
            'spacing.css': [
                { title: 'Typography', note: 'spacing.css', match: (key) => key === 'line-height-body' || key.startsWith('font-size-') },
                { title: 'Spacing', note: 'spacing.css', match: (key) => key.startsWith('gap-') || key.startsWith('pad-') && !key.startsWith('pad-button-') || key.startsWith('margin-') },
                { title: 'Radius', note: 'spacing.css', match: (key) => key.startsWith('radius-') && key !== 'radius-button' },
                { title: 'Buttons', note: 'spacing.css', match: (key) => key.startsWith('pad-button-') || key === 'radius-button' || key === 'font-size-button' },
                { title: 'Layout', note: 'spacing.css', match: (key) => key.startsWith('header-') || key.startsWith('main-') },
                { title: 'Heroes', note: 'spacing.css', match: (key) => key.startsWith('hero-') },
                { title: 'Navigation', note: 'spacing.css', match: (key) => key === 'font-size-nav' },
                { title: 'Embeds And Tables', note: 'spacing.css', match: (key) => key === 'table-cell-padding-y' }
            ]
        };

        const used = new Set();
        const sections = [];
        const definitions = sectionsByFile[filename] || [];

        for (const definition of definitions) {
            const sectionKeys = keys.filter((key) => !used.has(key) && definition.match(key));
            if (sectionKeys.length === 0) {
                continue;
            }

            sectionKeys.forEach((key) => used.add(key));
            sections.push({
                title: definition.title,
                note: definition.note,
                keys: sectionKeys.sort()
            });
        }

        const remaining = keys.filter((key) => !used.has(key)).sort();
        if (remaining.length > 0) {
            sections.push({
                title: 'Other',
                note: filename,
                keys: remaining
            });
        }

        return sections;
    },

    formatLabel: (key) => {
        return key
            .replace(/--/g, '')
            .replace(/-/g, ' ')
            .replace(/\b\w/g, (char) => char.toUpperCase());
    },

    renderControls: () => {
        themeEditor.renderFileSwitcher();
        themeEditor.controls.innerHTML = '';

        if (themeEditor.files.length === 0) {
            themeEditor.controls.textContent = "No CSS variables found in any theme file.";
            return;
        }

        const filename = themeEditor.activeFile || themeEditor.files[0];
        const vars = themeEditor.fileVars[filename];
        const sections = themeEditor.getSectionsForFile(filename, vars);

        if (!vars || Object.keys(vars).length === 0) {
            themeEditor.controls.textContent = "No CSS variables found in the selected file.";
            return;
        }

        const groupHeader = document.createElement('h3');
        groupHeader.className = 'control-file-title';
        groupHeader.textContent = filename;
        themeEditor.controls.appendChild(groupHeader);

        for (const section of sections) {
            const sectionWrap = document.createElement('section');
            sectionWrap.className = 'control-section';

            const sectionTitle = document.createElement('h4');
            sectionTitle.className = 'control-section-title';
            sectionTitle.textContent = section.title;
            sectionWrap.appendChild(sectionTitle);

            const sectionNote = document.createElement('p');
            sectionNote.className = 'control-section-note';
            sectionNote.textContent = `Stored in ${section.note}`;
            sectionWrap.appendChild(sectionNote);

            const gridDiv = document.createElement('div');
            gridDiv.className = 'controls-grid';
            sectionWrap.appendChild(gridDiv);

            for (const key of section.keys) {
                themeEditor.createControl(filename, key, vars[key], gridDiv);
            }

            themeEditor.controls.appendChild(sectionWrap);
        }
    },

    renderFileSwitcher: () => {
        themeEditor.fileSwitcher.innerHTML = '';

        const orderedFiles = [...themeEditor.files].sort((a, b) => {
            if (a === 'spacing.css') return -1;
            if (b === 'spacing.css') return 1;
            if (a === 'palette.css') return -1;
            if (b === 'palette.css') return 1;
            return a.localeCompare(b);
        });

        if (orderedFiles.length === 0) {
            themeEditor.fileSwitcher.textContent = "No editable theme files.";
            return;
        }

        for (const filename of orderedFiles) {
            const button = document.createElement('button');
            button.type = 'button';
            button.textContent = filename;
            button.setAttribute('aria-pressed', filename === themeEditor.activeFile ? 'true' : 'false');
            button.addEventListener('click', () => {
                themeEditor.activeFile = filename;
                themeEditor.renderControls();
            });
            themeEditor.fileSwitcher.appendChild(button);
        }
    },

    createControl: (filename, key, value, container) => {
        const div = document.createElement('div');
        div.className = 'control-group';

        const label = document.createElement('label');
        label.textContent = themeEditor.formatLabel(key);
        div.appendChild(label);

        const input = document.createElement('input');
        const isColor = /^#[0-9A-F]{6}$/i.test(value) || /^#[0-9A-F]{3}$/i.test(value);
        input.type = isColor ? 'color' : 'text';
        input.value = value;

        input.addEventListener('input', (e) => {
            const newValue = e.target.value;
            themeEditor.fileVars[filename][key] = newValue;
            themeEditor.updatePreview(key, newValue);
        });

        div.appendChild(input);
        container.appendChild(div);
    },

    saveTheme: async () => {
        if (!themeEditor.saveBtn || !themeEditor.controls) {
            return;
        }

        if (!confirm("Are you sure you want to save? This will override your current theme.")) {
            return;
        }

        themeEditor.saveBtn.textContent = "Saving...";
        let errors = 0;

        for (const filename of themeEditor.files) {
            const vars = themeEditor.fileVars[filename];
            if (!vars || Object.keys(vars).length === 0) {
                continue;
            }

            let content = themeEditor.fileContents[filename];
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
                if (!res.ok) {
                    throw new Error(`${res.status} ${res.statusText}: ${await res.text()}`);
                }
                themeEditor.fileContents[filename] = content;
            } catch (e) {
                console.error("Failed to save " + filename, e);
                errors++;
            }
        }

        if (errors === 0) {
            themeEditor.saveBtn.textContent = "Building...";
            try {
                const buildRes = await fetch('/api/project/build', { method: 'POST' });
                if (!buildRes.ok) {
                    throw new Error("Build failed");
                }
                if (themeEditor.preview) {
                    themeEditor.preview.src = themeEditor.preview.src;
                }
                themeEditor.saveBtn.textContent = "Saved";
                setTimeout(() => {
                    themeEditor.saveBtn.textContent = "Save Theme";
                }, 2000);
            } catch (e) {
                themeEditor.saveBtn.textContent = "Saved, build failed";
            }
        } else {
            themeEditor.saveBtn.textContent = "Save failed";
        }
    },

    updatePreview: (key, value) => {
        try {
            if (themeEditor.preview.contentDocument &&
                themeEditor.preview.contentDocument.documentElement) {
                themeEditor.preview.contentDocument.documentElement
                    .style.setProperty('--' + key, value);
            }
        } catch (e) {
            console.warn("Could not inject CSS directly", e);
        }
    }
};

window.themeEditor = themeEditor;
export { themeEditor };

document.addEventListener('DOMContentLoaded', () => {
    themeEditor.init();
});
