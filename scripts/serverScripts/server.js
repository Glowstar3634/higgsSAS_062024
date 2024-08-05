const express = require('express');
const bodyParser = require('body-parser');
const { exec } = require('child_process');
const app = express();
const port = 3000;

app.use(bodyParser.json());

app.post('/generate', (req, res) => {
    const outputFilePath = 'output.csv';
    exec(`./generateParticleData ${outputFilePath}`, (error, stdout, stderr) => {
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
