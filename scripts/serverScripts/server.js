const express = require('express');
const { exec } = require('child_process');
const path = require('path');
const app = express();
const port = 3000;

app.use(json());

app.post('/generate', (req, res) => {
    const outputFilePath = 'particleData1_01.csv';
    exec(`/home/azureuser/pythia8312/scripts/testScript2_06 ${outputFilePath}`, (error, stdout, stderr) => {
        if (error) {
            console.error(`Error: ${error.message}`);
            return res.status(500).send('Error generating data');
        }
        res.download(outputFilePath);
    });
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}/`);
});
