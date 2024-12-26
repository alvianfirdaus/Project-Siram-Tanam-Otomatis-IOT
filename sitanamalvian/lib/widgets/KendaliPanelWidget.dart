import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';

class KendaliPanelWidget extends StatefulWidget {
  final String selectedPlot;

  const KendaliPanelWidget({
    required this.selectedPlot,
  });

  @override
  _KendaliPanelWidgetState createState() => _KendaliPanelWidgetState();
}

class _KendaliPanelWidgetState extends State<KendaliPanelWidget> {
  bool isManualMode = false;
  bool isPumpOn = false;

  late DatabaseReference databaseReference;
  late DatabaseReference modeReference;
  late DatabaseReference pumpControlReference;

  @override
  void initState() {
    super.initState();

    // Initialize Firebase references
    databaseReference = FirebaseDatabase.instance.ref();
    modeReference = databaseReference.child('${widget.selectedPlot.toLowerCase()}/mode');
    pumpControlReference = databaseReference.child('${widget.selectedPlot.toLowerCase()}/manualPumpControl');

    // Listen to mode changes dynamically
    modeReference.onValue.listen((event) {
      final modeValue = event.snapshot.value as int? ?? 1; // Default to automatic mode if null
      setState(() {
        isManualMode = modeValue == 0; // 0 for manual mode
      });
    });

    // Listen to pump control changes dynamically
    pumpControlReference.onValue.listen((event) {
      final pumpControlValue = event.snapshot.value as int? ?? 0; // Default to pump off if null
      setState(() {
        isPumpOn = pumpControlValue == 1; // 1 means pump is on
      });
    });
  }

  // Update mode in Firebase
  void updateMode(bool manualMode) {
    databaseReference
        .child(widget.selectedPlot.toLowerCase())
        .update({
          'mode': manualMode ? 0 : 1, // 0 for manual, 1 for automatic
        })
        .then((_) {
          print('Mode updated successfully');
        })
        .catchError((error) {
          print('Failed to update mode: $error');
        });
  }

  // Update pump control in Firebase when in manual mode
  void updatePumpControl(bool pumpOn) {
    databaseReference
        .child(widget.selectedPlot.toLowerCase())
        .update({
          'manualPumpControl': pumpOn ? 1 : 0, // 1 for ON, 0 for OFF
        })
        .then((_) {
          print('Pump control updated successfully');
        })
        .catchError((error) {
          print('Failed to update pump control: $error');
        });
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(16),
        boxShadow: [
          BoxShadow(
            color: Colors.grey.withOpacity(0.3),
            spreadRadius: 2,
            blurRadius: 6,
            offset: Offset(0, 3),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Kendali IOT - ${widget.selectedPlot}',
            style: TextStyle(
              fontSize: 20,
              fontWeight: FontWeight.bold,
              color: Colors.green[900],
            ),
          ),
          SizedBox(height: 16),

          // Display current mode from Firebase
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                'Mode:',
                style: TextStyle(fontSize: 16, color: Colors.grey[700]),
              ),
              Row(
                children: [
                  ModeButton(
                    mode: 'Manual',
                    isSelected: isManualMode,
                    onTap: () {
                      updateMode(true); // Switch to manual mode
                    },
                  ),
                  SizedBox(width: 10),
                  ModeButton(
                    mode: 'Otomatis',
                    isSelected: !isManualMode,
                    onTap: () {
                      updateMode(false); // Switch to automatic mode
                    },
                  ),
                ],
              ),
            ],
          ),
          SizedBox(height: 16),

          // Show Pump ON/OFF button for manual mode, otherwise display status of automatic mode
          Center(
            child: isManualMode
                ? ElevatedButton(
                    onPressed: () {
                      updatePumpControl(!isPumpOn); // Toggle pump status
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: isPumpOn ? Colors.red : Colors.green,
                      padding: EdgeInsets.symmetric(horizontal: 40, vertical: 16),
                    ),
                    child: Text(
                      isPumpOn ? 'Matikan Pompa' : 'Hidupkan Pompa',
                      style: TextStyle(
                        fontSize: 18,
                        color: Colors.white,
                      ),
                    ),
                  )
                : Text(
                    'Mode Otomatis Aktif',
                    style: TextStyle(
                      fontSize: 18,
                      color: Colors.blue,
                    ),
                  ),
          ),
        ],
      ),
    );
  }
}

class ModeButton extends StatelessWidget {
  final String mode;
  final bool isSelected;
  final VoidCallback onTap;

  const ModeButton({
    required this.mode,
    required this.isSelected,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        decoration: BoxDecoration(
          color: isSelected ? Colors.orange : Colors.grey[300],
          borderRadius: BorderRadius.circular(16),
        ),
        child: Text(
          mode,
          style: TextStyle(
            fontSize: 16,
            color: isSelected ? Colors.white : Colors.black,
          ),
        ),
      ),
    );
  }
}
