channelID = 0000000; 
readAPIKey = 'YOUR_READ_API_KEY'; 
writeAPIKey = 'YOUR_WRITE_API_KEY';


try
    % Reading the data stream as a table structure
    data = thingSpeakRead(channelID, 'Fields', [1, 3], 'NumPoints', 1, 'ReadKey', readAPIKey, 'OutputFormat', 'table');

    if ~isempty(data)
        val1 = data{1, 2}; % Column 2 is Field 1 (Encrypted Hex String)
        val3 = data{1, 3}; % Column 3 is Field 3 (Quantum Key)
        
        % extraction for Hex Ciphertext
        if iscell(val1), hexCipher = char(val1{1}); else, hexCipher = char(val1); end
        
        %  If the key arrives as a number, convert it to text
        if isnumeric(val3)
            quantumKey = num2str(val3);
        elseif iscell(val3)
            quantumKey = char(val3{1});
        else
            quantumKey = char(val3);
        end

        fprintf('DEBUG - Raw Hex Received from ESP32: %s\n', hexCipher);
        fprintf('DEBUG - Quantum Key Used: %s\n', quantumKey);

        % 2. Convert HEX string back to raw bytes
        rawBytes = sscanf(hexCipher, '%2x')';

        if ~isempty(rawBytes) && ~isempty(quantumKey)
            % 3. Execute the Symmetrical Bitwise XOR Decryption Cipher
            decryptedChars = zeros(1, length(rawBytes));
            for i = 1:length(rawBytes)
                keyChar = quantumKey(mod(i-1, length(quantumKey)) + 1);
                decryptedChars(i) = bitxor(rawBytes(i), double(keyChar));
            end

            % 4. Convert characters back to a readable decimal temperature number
            decryptedTempString = char(decryptedChars);
            fprintf('DEBUG - What it looked like after Decryption: %s\n', decryptedTempString);
            
            decryptedTemperature = str2double(decryptedTempString);

            % 5. Post decrypted plain text temperature directly into Field 2!
            if ~isnan(decryptedTemperature)
                try
                    thingSpeakWrite(channelID, 'Fields', 2, 'Values', decryptedTemperature, 'WriteKey', writeAPIKey);
                    fprintf('Decryption Successful! Temperature recovered: %f\n', decryptedTemperature);
                catch writeError
                    fprintf('Write deferred (rate limit lockout active). Will auto-retry on next cycle.\n');
                end
            else
                fprintf('Data format alert: Decrypted string layout could not form a number.\n');
            end
        else
            fprintf('Payload check: Hex cipher or quantum key strings are currently empty.\n');
        end
    else
        fprintf('Status: Waiting for the initial ESP32 edge node upload stream...\n');
    end
catch generalError
    fprintf('System Exception caught: %s\n', generalError.message);
end