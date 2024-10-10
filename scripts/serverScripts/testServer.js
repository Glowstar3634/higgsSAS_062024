const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const app = express();
const port = 8080;

app.get('/generate', (req, res) => {
    const child = spawn('/home/ubuntu/pythia8312/scripts/t9pgen6.05', ['/home/ubuntu/pythia8312/scripts/particleData6_05.csv']);

    child.stdout.on('data', (data) => {
        console.log(`stdout: ${data}`);
    });

    child.stderr.on('data', (data) => {
        console.error(`stderr: ${data}`);
    });

    child.on('error', (error) => {
        console.error(`exec error: ${error}`);
        res.status(500).send(`Error: ${error.message}`);
    });

    child.on('close', (code) => {
        console.log(`Child process exited with code ${code}`);
        if (code === 0) {
            res.sendFile(path.resolve('/home/ubuntu/pythia8312/scripts/particleData6_05.csv'));
        } else {
            res.status(500).send(`Process failed with exit code ${code}`);
        }
    });
});

const server = app.listen(port, '0.0.0.0', () => {
    console.log(`Server running at http://0.0.0.0:${port}/`);
});
server.setTimeout(2400000);
