//Testing...
const express = require('express');
const { exec } = require('child_process');
const path = require('path');
const app = express();
const port = 3000;

app.get('/generate', (req, res) => {
    exec('/home/azureuser/pythia8312/scripts/testScript2_07 /home/azureuser/pythia8312/scripts/particleData2_07.csv', { maxBuffer: 5 * 1024 * 1024 }, (error, stdout, stderr) => {
        if (error) {
            console.error(`exec error: ${error}`);
            res.status(500).send(`Error: ${error.message}`);
            return;
        }
        if (stderr) {
            console.error(`stderr: ${stderr}`);
            res.status(500).send(`Error: ${stderr}`);
            return;
        }
        console.log(`stdout: ${stdout}`);
        res.sendFile(path.resolve('/home/azureuser/pythia8312/scripts/particleData2_07.csv'));
    });
});

app.listen(port, '0.0.0.0', () => {
    console.log(`Server running at http://0.0.0.0:${port}/`);
});