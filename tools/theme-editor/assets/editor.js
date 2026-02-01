
export async function Init() {
    console.log("Initializing Theme Editor...");
    const controls = document.getElementById('controls');
    const preview = document.getElementById('preview-frame');
    const frameWrapper = document.getElementById('frame-wrapper');
    const saveBtn = document.getElementById('save-btn');
    const mobileBtn = document.getElementById('mobile-btn');

    // 1. Load Theme
    let theme = {};
    try {
        const res = await fetch('/api/theme');
        theme = await res.json();
    } catch (e) {
        console.error("Failed to load theme", e);
    }

    // 2. Render Controls
    controls.innerHTML = '';

    const createControl = (key, value) => {
        const div = document.createElement('div');
        div.className = 'control-group';

        const label = document.createElement('label');
        label.innerText = key.replace(/_/g, ' ').toUpperCase();
        div.appendChild(label);

        const input = document.createElement('input');
        if (value.startsWith('#')) {
            input.type = 'color';
        } else {
            input.type = 'text';
        }
        input.value = value;

        input.addEventListener('input', (e) => {
            theme[key] = e.target.value;
            UpdatePreview(key, e.target.value);
        });

        div.appendChild(input);
        controls.appendChild(div);
    };

    for (const [key, value] of Object.entries(theme)) {
        createControl(key, value);
    }

    // 3. Save Handler
    saveBtn.addEventListener('click', async () => {
        try {
            const res = await fetch('/api/theme', {
                method: 'POST',
                body: JSON.stringify(theme)
            });
            const result = await res.json();
            if (result.success) {
                alert('Theme Saved!');
            } else {
                alert('Save Failed');
            }
        } catch (e) {
            console.error(e);
            alert('Error saving theme');
        }
    });

    // 4. Mobile Toggle
    let isMobile = false;
    mobileBtn.addEventListener('click', () => {
        isMobile = !isMobile;
        // Adjust iframe container width
        if (isMobile) {
            frameWrapper.style.width = '375px';
            frameWrapper.style.margin = '0 auto';
            frameWrapper.style.border = '10px solid #333';
            frameWrapper.style.borderRadius = '20px';
        } else {
            frameWrapper.style.width = '100%';
            frameWrapper.style.margin = '0';
            frameWrapper.style.border = 'none';
            frameWrapper.style.borderRadius = '0';
        }
    });

    // 5. Update Preview & Self Logic
    const UpdatePreview = (key, value) => {
        const cssVar = '--' + key.replace(/_/g, '-');

        // A. Update Editor Itself (Dynamic CSS)
        document.documentElement.style.setProperty(cssVar, value);

        // B. Update Iframe
        try {
            if (preview.contentDocument && preview.contentDocument.documentElement) {
                preview.contentDocument.documentElement.style.setProperty(cssVar, value);
            }
        } catch (e) {
            console.warn("Could not inject CSS directly involved CORS or load issue", e);
        }
    };

    // Initial apply
    preview.onload = () => {
        for (const [key, value] of Object.entries(theme)) {
            UpdatePreview(key, value);
        }
    };

    // Also apply to self immediately
    for (const [key, value] of Object.entries(theme)) {
        UpdatePreview(key, value);
    }
}

export function SaveTheme() {
    document.getElementById('save-btn').click();
}
