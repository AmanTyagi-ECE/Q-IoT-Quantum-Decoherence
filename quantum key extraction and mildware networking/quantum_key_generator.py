from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator

def generate_quantum_key():

    # the ideal quantum state
 
    qc = QuantumCircuit(2)
    qc.h(0)           # Put Qubit 0 into superposition
    qc.cx(0, 1)       # Entangle Qubit 0 and Qubit 1
    
    # measurement (wave function collapse)
    qc.measure_all() 


    # We use AerSimulator to mimic a real quantum computer's memory
    simulator = AerSimulator()
    
    # Run the circuit EXACTLY once (shots=1) to get one definitive key
    job = simulator.run(qc, shots=1)
    result = job.result()
    
   
    # get_counts() returns a dictionary like: {'11': 1} or {'00': 1}
    counts = result.get_counts(qc)
    
    # Extract just the string ('11' or '00') from the dictionary keys
    quantum_key = list(counts.keys())[0]
    
    return quantum_key

# test block
if __name__ == "__main__":
    final_key = generate_quantum_key()
    print(f"Wave function collapsed. Quantum Key: {final_key}")