const projectEditor = {
    tree: null,
    editor: null,
    status: null,
    root: null,
    path: null,
    saveBtn: null,
    compileBtn: null,
    currentFile: '',
    currentButton: null,
    dirty: false,
    files: [],
    sourcePath: '',
    treeSnapshot: '[]',
    syncHandle: null,
    syncPending: false,

    init: async () => {
        projectEditor.tree = document.getElementById('project-tree');
        projectEditor.editor = document.getElementById('editor-text');
        projectEditor.status = document.getElementById('ide-status');
        projectEditor.root = document.getElementById('ide-project-root');
        projectEditor.path = document.getElementById('editor-path');
        projectEditor.saveBtn = document.getElementById('save-file');
        projectEditor.compileBtn = document.getElementById('compile-project');

        if (!projectEditor.tree || !projectEditor.editor || !projectEditor.status || !projectEditor.path) {
            return;
        }

        projectEditor.setEditorEnabled(false);

        projectEditor.editor.addEventListener('input', () => {
            if (!projectEditor.currentFile) {
                return;
            }

            projectEditor.dirty = true;
            projectEditor.updatePath();
            projectEditor.setStatus(`Editing ${projectEditor.currentFile} (unsaved)`);
        });

        document.addEventListener('keydown', (event) => {
            if ((event.metaKey || event.ctrlKey) && event.key.toLowerCase() === 's') {
                event.preventDefault();
                projectEditor.saveFile();
            }
        });

        window.addEventListener('focus', () => {
            projectEditor.syncFileTree(true);
        });

        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                projectEditor.syncFileTree(true);
            }
        });

        await projectEditor.loadProjectStatus();
        await projectEditor.syncFileTree();

        projectEditor.syncHandle = window.setInterval(() => {
            projectEditor.syncFileTree(true);
        }, 2500);
    },

    setStatus: (message) => {
        if (projectEditor.status) {
            projectEditor.status.textContent = message;
        }
    },

    setProjectRoot: (message) => {
        if (projectEditor.root) {
            projectEditor.root.textContent = `Editing source: ${message}`;
        }
    },

    updatePath: () => {
        if (!projectEditor.path) {
            return;
        }

        if (!projectEditor.currentFile) {
            projectEditor.path.textContent = 'No file selected.';
            return;
        }

        const suffix = projectEditor.dirty ? ' *' : '';
        projectEditor.path.textContent = `${projectEditor.currentFile}${suffix}`;
    },

    setEditorEnabled: (enabled) => {
        if (!projectEditor.editor) {
            return;
        }

        projectEditor.editor.disabled = !enabled;
        if (!enabled) {
            projectEditor.editor.value = '';
        }
    },

    setCurrentButton: (button) => {
        if (projectEditor.currentButton) {
            projectEditor.currentButton.setAttribute('aria-current', 'false');
        }

        projectEditor.currentButton = button;

        if (projectEditor.currentButton) {
            projectEditor.currentButton.setAttribute('aria-current', 'true');
        }
    },

    loadProjectStatus: async () => {
        try {
            const res = await fetch('/api/project/status');
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            const status = await res.json();
            projectEditor.sourcePath = status.sourcePath || status.path || '';
            projectEditor.setProjectRoot(projectEditor.sourcePath);
        } catch (e) {
            console.error('Failed to load project status', e);
            projectEditor.setProjectRoot('Project root unavailable.');
        }
    },

    syncFileTree: async (silent = false) => {
        if (projectEditor.syncPending) {
            return;
        }

        projectEditor.syncPending = true;

        if (!silent) {
            projectEditor.setStatus('Loading project files...');
        }

        try {
            const res = await fetch('/api/project/files');
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            const files = await res.json();
            const snapshot = JSON.stringify(files);
            const changed = snapshot !== projectEditor.treeSnapshot;

            projectEditor.files = files;
            projectEditor.treeSnapshot = snapshot;

            if (changed || !silent) {
                projectEditor.renderFileTree();
            }

            if (projectEditor.currentFile && !projectEditor.files.includes(projectEditor.currentFile)) {
                projectEditor.currentFile = '';
                projectEditor.currentButton = null;
                projectEditor.dirty = false;
                projectEditor.setEditorEnabled(false);
                projectEditor.updatePath();
                projectEditor.setStatus('The selected file is no longer available.');
                projectEditor.syncPending = false;
                return;
            }

            if (!silent) {
                projectEditor.setStatus('Project files loaded.');
            }
        } catch (e) {
            console.error('Failed to load project files', e);
            projectEditor.tree.textContent = 'Unable to load project files.';
            projectEditor.setStatus('Project files unavailable.');
        } finally {
            projectEditor.syncPending = false;
        }
    },

    refreshTree: async () => {
        await projectEditor.loadProjectStatus();
        await projectEditor.syncFileTree();
    },

    buildTree: (paths) => {
        const root = { name: '', kind: 'directory', children: new Map() };

        for (const filePath of paths) {
            const parts = filePath.split('/').filter(Boolean);
            let current = root;
            let currentPath = '';

            for (let i = 0; i < parts.length; i += 1) {
                const part = parts[i];
                currentPath = currentPath ? `${currentPath}/${part}` : part;
                const isFile = i === parts.length - 1;

                if (!current.children.has(part)) {
                    current.children.set(part, {
                        name: part,
                        path: currentPath,
                        kind: isFile ? 'file' : 'directory',
                        children: isFile ? null : new Map()
                    });
                }

                current = current.children.get(part);
            }
        }

        return root;
    },

    sortNodes: (nodes) => {
        return [...nodes].sort((a, b) => {
            if (a.kind !== b.kind) {
                return a.kind === 'directory' ? -1 : 1;
            }
            return a.name.localeCompare(b.name);
        });
    },

    renderNode: (node, depth = 0) => {
        if (node.kind === 'file') {
            const button = document.createElement('button');
            button.type = 'button';
            button.className = 'project-tree-file';
            button.textContent = node.name;
            button.dataset.path = node.path;
            button.setAttribute('aria-current', node.path === projectEditor.currentFile ? 'true' : 'false');
            if (node.path === projectEditor.currentFile) {
                projectEditor.currentButton = button;
            }
            button.addEventListener('click', () => {
                projectEditor.openFile(node.path, button);
            });
            return button;
        }

        const wrap = document.createElement('div');
        wrap.className = 'project-tree-folder';

        const details = document.createElement('details');
        details.open = depth < 2;

        const summary = document.createElement('summary');
        summary.textContent = node.name;
        details.appendChild(summary);

        const children = document.createElement('div');
        children.className = 'project-tree-children';

        for (const child of projectEditor.sortNodes(node.children.values())) {
            children.appendChild(projectEditor.renderNode(child, depth + 1));
        }

        details.appendChild(children);
        wrap.appendChild(details);
        return wrap;
    },

    renderFileTree: () => {
        projectEditor.tree.innerHTML = '';
        projectEditor.currentButton = null;

        if (projectEditor.files.length === 0) {
            projectEditor.tree.textContent = 'No editable project files found.';
            return;
        }

        const root = projectEditor.buildTree(projectEditor.files);
        const rootWrap = document.createElement('div');
        rootWrap.className = 'project-tree-root';

        for (const child of projectEditor.sortNodes(root.children.values())) {
            rootWrap.appendChild(projectEditor.renderNode(child));
        }

        projectEditor.tree.appendChild(rootWrap);
    },

    findFileButton: (filePath) => {
        return [...projectEditor.tree.querySelectorAll('[data-path]')].find((button) => {
            return button.dataset.path === filePath;
        }) || null;
    },

    confirmAbandonChanges: async () => {
        if (!projectEditor.dirty) {
            return true;
        }

        return confirm('You have unsaved changes. Continue without saving?');
    },

    openFile: async (filePath, button) => {
        if (filePath === projectEditor.currentFile) {
            projectEditor.setCurrentButton(button);
            return;
        }

        const shouldContinue = await projectEditor.confirmAbandonChanges();
        if (!shouldContinue) {
            return;
        }

        projectEditor.setStatus(`Opening ${filePath}...`);

        try {
            const res = await fetch('/api/project/file/read', {
                headers: { 'X-File': filePath }
            });
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            const text = await res.text();
            projectEditor.currentFile = filePath;
            projectEditor.setEditorEnabled(true);
            projectEditor.editor.value = text;
            projectEditor.dirty = false;
            projectEditor.updatePath();
            projectEditor.setCurrentButton(button || projectEditor.findFileButton(filePath));
            projectEditor.setStatus(`Editing ${filePath}`);
        } catch (e) {
            console.error(`Failed to open ${filePath}`, e);
            projectEditor.setStatus(`Failed to open ${filePath}`);
        }
    },

    saveFile: async () => {
        if (!projectEditor.currentFile) {
            projectEditor.setStatus('Choose a file before saving.');
            return;
        }

        projectEditor.setStatus(`Saving ${projectEditor.currentFile}...`);

        try {
            const res = await fetch('/api/project/file/write', {
                method: 'POST',
                headers: { 'X-File': projectEditor.currentFile },
                body: projectEditor.editor.value
            });
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            projectEditor.dirty = false;
            projectEditor.updatePath();
            projectEditor.setStatus(`Saved ${projectEditor.currentFile}`);
        } catch (e) {
            console.error(`Failed to save ${projectEditor.currentFile}`, e);
            projectEditor.setStatus(`Failed to save ${projectEditor.currentFile}`);
        }
    },

    createFile: async () => {
        const requestedPath = prompt('Enter a new file path relative to the source folder:');
        if (!requestedPath) {
            return;
        }

        const filePath = requestedPath.trim().replace(/^\/+/, '');
        if (!filePath) {
            projectEditor.setStatus('Enter a valid file path.');
            return;
        }

        projectEditor.setStatus(`Creating ${filePath}...`);

        try {
            const res = await fetch('/api/project/file/create', {
                method: 'POST',
                headers: { 'X-File': filePath },
                body: ''
            });
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            await projectEditor.syncFileTree(true);
            await projectEditor.openFile(filePath, projectEditor.findFileButton(filePath));
            projectEditor.setStatus(`Created ${filePath}`);
        } catch (e) {
            console.error(`Failed to create ${filePath}`, e);
            projectEditor.setStatus(`Failed to create ${filePath}`);
        }
    },

    deleteFile: async () => {
        if (!projectEditor.currentFile) {
            projectEditor.setStatus('Choose a file before deleting.');
            return;
        }

        if (!confirm(`Delete ${projectEditor.currentFile}?`)) {
            return;
        }

        const filePath = projectEditor.currentFile;
        projectEditor.setStatus(`Deleting ${filePath}...`);

        try {
            const res = await fetch('/api/project/file/delete', {
                method: 'POST',
                headers: { 'X-File': filePath }
            });
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            projectEditor.currentFile = '';
            projectEditor.currentButton = null;
            projectEditor.dirty = false;
            projectEditor.setEditorEnabled(false);
            projectEditor.updatePath();
            await projectEditor.syncFileTree(true);
            projectEditor.setStatus(`Deleted ${filePath}`);
        } catch (e) {
            console.error(`Failed to delete ${filePath}`, e);
            projectEditor.setStatus(`Failed to delete ${filePath}`);
        }
    },

    compileProject: async () => {
        if (projectEditor.dirty) {
            await projectEditor.saveFile();
            if (projectEditor.dirty) {
                return;
            }
        }

        projectEditor.setStatus('Compiling project...');

        try {
            const res = await fetch('/api/project/build', { method: 'POST' });
            if (!res.ok) {
                throw new Error(res.statusText);
            }

            await projectEditor.syncFileTree(true);
            projectEditor.setStatus('Compile complete.');
        } catch (e) {
            console.error('Build failed', e);
            projectEditor.setStatus('Compile failed.');
        }
    },

    openPreview: () => {
        window.open('/preview/index.html', '_blank');
    }
};

window.projectEditor = projectEditor;
export { projectEditor };

document.addEventListener('DOMContentLoaded', () => {
    projectEditor.init();
});
