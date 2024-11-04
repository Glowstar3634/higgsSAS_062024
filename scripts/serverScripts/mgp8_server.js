const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const app = express();
const port = 8080;

app.get('/generate', (req, res) => {
    // Step 1: Generate LHE file with MadGraph
    const madGraphCommand = '/home/ubuntu/MG5_aMC_v3_6_0/scripts/madgraph1.txt'; // Update this path
    const outputZip = '/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run3/Events/run_01/unweighted_events.lhe.gz'; // Update this path
    const outputLHE = '/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run3/Events/run_01/unweighted_events.lhe'; // Update this path for unzipped LHE
    const pythiaOutput = '/home/ubuntu/pythia8312/scripts/particleData7_02.csv'; // Update this path

    const madGraphChild = spawn('bash', [madGraphCommand]);

    madGraphChild.stdout.on('data', (data) => {
        console.log(`MadGraph stdout: ${data}`);
    });

    madGraphChild.stderr.on('data', (data) => {
        console.error(`MadGraph stderr: ${data}`);
    });

    madGraphChild.on('error', (error) => {
        console.error(`MadGraph exec error: ${error}`);
        res.status(500).send(`Error: ${error.message}`);
    });

    madGraphChild.on('close', (code) => {
        console.log(`MadGraph process exited with code ${code}`);
        if (code === 0) {
            // Step 2: Unzip the LHE file
            const unzipCommand = `gunzip ${outputZip}`;
            const unzipChild = spawn('bash', ['-c', unzipCommand]);

            unzipChild.on('close', (unzipCode) => {
                console.log(`Unzip process exited with code ${unzipCode}`);
                if (unzipCode === 0) {
                    // Step 3: Run Pythia script on the generated LHE file
                    const pythiaChild = spawn('bash', ['/home/ubuntu/pythia8312/scripts/pgen7.02', outputLHE, pythiaOutput]);

                    pythiaChild.stdout.on('data', (data) => {
                        console.log(`Pythia stdout: ${data}`);
                    });

                    pythiaChild.stderr.on('data', (data) => {
                        console.error(`Pythia stderr: ${data}`);
                    });

                    pythiaChild.on('error', (error) => {
                        console.error(`Pythia exec error: ${error}`);
                        res.status(500).send(`Error: ${error.message}`);
                    });

                    pythiaChild.on('close', (pythiaCode) => {
                        console.log(`Pythia process exited with code ${pythiaCode}`);
                        if (pythiaCode === 0) {
                            res.sendFile(path.resolve(pythiaOutput));
                        } else {
                            res.status(500).send(`Pythia process failed with exit code ${pythiaCode}`);
                        }
                    });
                } else {
                    res.status(500).send(`Unzip process failed with exit code ${unzipCode}`);
                }
            });
        } else {
            res.status(500).send(`MadGraph process failed with exit code ${code}`);
        }
    });
});

const server = app.listen(port, '0.0.0.0', () => {
    console.log(`Server running at http://0.0.0.0:${port}/`);
});
server.setTimeout(2400000);
