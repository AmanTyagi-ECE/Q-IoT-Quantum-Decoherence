from flask import Flask, jsonify
# import the function from the file quantum key generator file
from quantum_key_generator import generate_quantum_key

# Initialize the Flask application
app = Flask(__name__)

# This creates URL endpoint
@app.route('/get_key', methods=['GET'])
def get_key_endpoint():
    try:
        # creating new fresh key 
        new_key = generate_quantum_key()
        
        #  Packaing it in JSON format
        response_data = {
            "status": "success",
            "quantum_key": new_key,
            "message": "Wave function collapsed successfully."
        }
        
        return jsonify(response_data), 200
        
    except Exception as e:
        # Just in case the physics engine crashes!
        return jsonify({"status": "error", "message": str(e)}), 500

# This turns the server on and opens it to your local Wi-Fi
if __name__ == '__main__':
    print("Starting Q-IoT Flask Server...")
    # host='0.0.0.0' allows other devices (like the ESP32) to connect
    # port=5000 is the standard Flask door number
    app.run(host='0.0.0.0', port=5000, debug=True)