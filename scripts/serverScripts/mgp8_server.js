const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const app = express();
const port = 8080;

function loadWilsonCoefficients(callback) {
    fs.readFile('wilson_coefficients.json', 'utf8', (err, data) => {
        if (err) {
            console.error('Error loading Wilson coefficients:', err);
            callback(err, null);
            return;
        }
        const coefficients = JSON.parse(data);
        callback(null, coefficients);
    });
}

app.get('/generate', (req, res) => {
    const loopIterations = 10;  // Loop 10 times

    // Recursive function to run the process 10 times
    const runGenerationLoop = (iteration) => {
        if (iteration > loopIterations) {
            return;  // Exit once all iterations are completed
        }

        console.log(`Starting iteration ${iteration}`);

        // Step 1: Run MadGraph1 script
        const madGraphChild1 = spawn('bash', ['-c', 'cd /home/ubuntu/MG5_aMC_v3_6_0 && ./bin/mg5_aMC /home/ubuntu/MG5_aMC_v3_6_0/scripts/madgraph1.txt']);
        
        madGraphChild1.stdout.on('data', (data) => {
            console.log(`MadGraph1 stdout: ${data}`);
        });

        madGraphChild1.stderr.on('data', (data) => {
            console.error(`MadGraph1 stderr: ${data}`);
        });

        madGraphChild1.on('error', (error) => {
            console.error(`MadGraph1 exec error: ${error}`);
            res.status(500).send(`Error: ${error.message}`);
        });

        madGraphChild1.on('close', (code) => {
            console.log(`MadGraph1 process exited with code ${code}`);
            if (code === 0) {
                // Step 2: Update Coefficients with Python script
                const updateCoefficientsChild = spawn('python3', ['/home/ubuntu/pythia8312/scripts/coefficientUpdate.py']);

                updateCoefficientsChild.stdout.on('data', (data) => {
                    console.log(`Coefficient update stdout: ${data}`);
                });

                updateCoefficientsChild.stderr.on('data', (data) => {
                    console.error(`Coefficient update stderr: ${data}`);
                });

                updateCoefficientsChild.on('error', (error) => {
                    console.error(`Coefficient update exec error: ${error}`);
                    res.status(500).send(`Error: ${error.message}`);
                });

                updateCoefficientsChild.on('close', (code) => {
                    console.log(`Coefficient update process exited with code ${code}`);
                    if (code === 0) {
                        // Step 3: Run MadGraph2 script
                        const madGraphChild2 = spawn('bash', ['-c', 'cd /home/ubuntu/MG5_aMC_v3_6_0 && ./bin/mg5_aMC /home/ubuntu/MG5_aMC_v3_6_0/scripts/madgraph2.txt']);

                        madGraphChild2.stdout.on('data', (data) => {
                            console.log(`MadGraph2 stdout: ${data}`);
                        });

                        madGraphChild2.stderr.on('data', (data) => {
                            console.error(`MadGraph2 stderr: ${data}`);
                        });

                        madGraphChild2.on('error', (error) => {
                            console.error(`MadGraph2 exec error: ${error}`);
                            res.status(500).send(`Error: ${error.message}`);
                        });

                        madGraphChild2.on('close', (code) => {
                            console.log(`MadGraph2 process exited with code ${code}`);
                            if (code === 0) {
                                // Step 4: Unzip the generated LHE file
                                const outputZip = '/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run3/Events/run_01/unweighted_events.lhe.gz';
                                const outputLHE = '/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run3/Events/run_01/unweighted_events.lhe';
                                const unzipCommand = `gunzip ${outputZip}`;
                                const unzipChild = spawn('bash', ['-c', unzipCommand]);

                                unzipChild.on('close', (unzipCode) => {
                                    console.log(`Unzip process exited with code ${unzipCode}`);
                                    if (unzipCode === 0) {
                                        // Step 5: Run Pythia script for showering
                                        const pythiaOutput = '/home/ubuntu/pythia8312/scripts/particleData7_02.csv';
                                        const pythiaChild = spawn('/home/ubuntu/pythia8312/scripts/pgen7.02', [outputLHE, pythiaOutput]);

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
                                                // Step 6: Load Wilson coefficients and append to the Pythia output
                                                loadWilsonCoefficients((err, wilsonCoefficients) => {
                                                    if (err) {
                                                        return res.status(500).send(`Error loading Wilson coefficients: ${err.message}`);
                                                    }
                                        
                                                    fs.readFile(pythiaOutput, 'utf8', (err, data) => {
                                                        if (err) {
                                                            console.error('Error reading Pythia output CSV:', err);
                                                            return res.status(500).send(`Error: ${err.message}`);
                                                        }
                                        
                                                        const wilsonString = `# Wilson Coefficients: ${Object.entries(wilsonCoefficients).map(([key, value]) => `${key}: ${value}`).join(', ')}\n`;
                                                        const newCsvData = wilsonString + data;
                                                        const finalOutput = path.join('/home/ubuntu/pythia8312/scripts/', `particleData7_02_with_coeffs.csv`);
                                        
                                                        fs.writeFile(finalOutput, newCsvData, (err) => {
                                                            if (err) {
                                                                console.error('Error writing updated CSV file:', err);
                                                                return res.status(500).send(`Error: ${err.message}`);
                                                            }
                                        
                                                            // Send file
                                                            res.sendFile(finalOutput, () => {
                                                                console.log(`Sent dataset for iteration ${iteration}`);
                                                                console.log(`Proceeding to iteration ${iteration + 1}`); // Debugging log
                                                                setTimeout(() => {
                                                                    runGenerationLoop(iteration + 1); // Proceed to the next iteration
                                                                }, 5000); // Delay for 5 seconds to ensure previous iteration has completed                                                                
                                                            });
                                                        });
                                                    });
                                                });
                                            } else {
                                                res.status(500).send(`Pythia process failed with exit code ${pythiaCode}`);
                                            }
                                        });
                                        
                                    } else {
                                        res.status(500).send(`Unzip process failed with exit code ${unzipCode}`);
                                    }
                                });
                            } else {
                                res.status(500).send(`MadGraph2 process failed with exit code ${code}`);
                            }
                        });
                    } else {
                        res.status(500).send(`Coefficient update process failed with exit code ${code}`);
                    }
                });
            } else {
                res.status(500).send(`MadGraph1 process failed with exit code ${code}`);
            }
        });
    };

    runGenerationLoop(1);
});

const server = app.listen(port, '0.0.0.0', () => {
    console.log(`Server running at http://0.0.0.0:${port}/ \nReady to generate SMEFT datasets.`);
});
server.setTimeout(18000000);
