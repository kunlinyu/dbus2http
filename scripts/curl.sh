curl -X POST http://localhost:8080/dbus/com.example.ServiceName/path/to/object/com.example.InterfaceName.Method2 -H "Content-Type: application/json" -d '{
  "name": "Charlie",
  "name2age": {
    "Alice": 17,
    "Bob": 18
  },
  "name2valid": {
    "Alice": true,
    "Bob": false
  },
  "num": 123,
  "valid": true
}'
