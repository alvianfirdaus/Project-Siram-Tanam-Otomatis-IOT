import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';
import 'dart:async';
import 'package:sitanamalvian/Routes/routes.dart';

class CatatanScreen extends StatefulWidget {
  @override
  _CatatanScreenState createState() => _CatatanScreenState();
}

class _CatatanScreenState extends State<CatatanScreen> {
  String selectedPlot = "plot1";
  Map<String, dynamic> plotData = {};
  final DatabaseReference databaseReference = FirebaseDatabase.instance.ref();
  StreamSubscription? plotSubscription;

  void listenToPlotData(String plot) {
    // Membatalkan subscription sebelumnya jika ada
    plotSubscription?.cancel();

    plotSubscription = databaseReference.child(plot.toLowerCase()).onValue.listen((event) {
      if (event.snapshot.value != null) {
        final data = event.snapshot.value;
        if (data is Map<Object?, Object?>) {
          setState(() {
            plotData = data.map(
              (key, value) => MapEntry(key.toString(), value),
            );
          });
        } else {
          setState(() {
            plotData = {};
          });
        }
      } else {
        setState(() {
          plotData = {};
        });
      }
    });
  }

  @override
  void initState() {
    super.initState();
    listenToPlotData(selectedPlot);
  }

  @override
  void dispose() {
    // Membatalkan subscription saat widget dihancurkan
    plotSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Stack(
        children: [
          // Background image
          Container(
            height: 230,
            width: double.infinity,
            decoration: BoxDecoration(
              image: DecorationImage(
                image: AssetImage('assets/images/headercatatann.png'),
                fit: BoxFit.cover,
              ),
            ),
          ),

          // Plot selection buttons overlaid on the image
          Positioned(
            top: 130,
            left: 0,
            right: 0,
            child: Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                PlotBox(
                  plotName: "plot1",
                  isSelected: selectedPlot == "plot1",
                  onTap: () {
                    setState(() {
                      selectedPlot = "plot1";
                      listenToPlotData("plot1");
                    });
                  },
                ),
                SizedBox(width: 16),
                PlotBox(
                  plotName: "plot2",
                  isSelected: selectedPlot == "plot2",
                  onTap: () {
                    setState(() {
                      selectedPlot = "plot2";
                      listenToPlotData("plot2");
                    });
                  },
                ),
              ],
            ),
          ),

          // Main content below the image
          Column(
            children: [
              SizedBox(height: 230), // Space for the image

              // List of notes
              Expanded(
                child: plotData.isNotEmpty && plotData["zcatatan"] != null
                    ? ListView.builder(
                        itemCount: (plotData["zcatatan"] as Map).keys.length,
                        itemBuilder: (context, index) {
                          var sortedKeys = (plotData["zcatatan"] as Map).keys.toList()
                            ..sort((a, b) {
                              return int.parse(b).compareTo(int.parse(a));
                            });

                          String key = sortedKeys[index];
                          Map<String, dynamic> catatan = (plotData["zcatatan"][key] as Map).cast<String, dynamic>();

                          return CatatanCard(
                            tanggal: catatan["tanggal"] ?? "",
                            waktu: catatan["waktu"] ?? "",
                            catatan: catatan["catatan"] ?? "",
                            onDelete: () async {
                              try {
                                // Menghapus data dari Firebase
                                await databaseReference.child('$selectedPlot/zcatatan/$key').remove();
                              } catch (e) {
                                print("Terjadi kesalahan saat menghapus data: $e");
                              }
                            },
                          );
                        },
                      )
                    : Center(
                        child: Text("Belum ada catatan."),
                      ),
              ),
            ],
          ),
        ],
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {
          // Pindah ke halaman tambah catatan
          Navigator.pushNamed(context, '/addcatatan', arguments: selectedPlot);
        },
        child: Icon(Icons.add),
      ),
    );
  }
}

class PlotBox extends StatelessWidget {
  final bool isSelected;
  final String plotName;
  final VoidCallback onTap;

  const PlotBox({
    required this.isSelected,
    required this.plotName,
    required this.onTap,
    Key? key,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Column(
        children: [
          Container(
            width: 50,
            height: 50,
            decoration: BoxDecoration(
              color: isSelected ? Colors.orange : Colors.white,
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
            child: Icon(
              Icons.local_florist,
              color: isSelected ? Colors.white : Colors.green,
              size: 30,
            ),
          ),
          SizedBox(height: 8),
          Text(
            plotName,
            style: TextStyle(
              color: isSelected ? Colors.orange : Colors.green,
              fontSize: 12,
            ),
          ),
        ],
      ),
    );
  }
}

class CatatanCard extends StatelessWidget {
  final String tanggal;
  final String waktu;
  final String catatan;
  final VoidCallback onDelete;

  const CatatanCard({
    required this.tanggal,
    required this.waktu,
    required this.catatan,
    required this.onDelete,
    Key? key,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Dismissible(
      key: UniqueKey(),
      direction: DismissDirection.startToEnd,
      confirmDismiss: (direction) async {
        return await showDialog(
          context: context,
          builder: (BuildContext context) {
            return AlertDialog(
              title: Text("Konfirmasi"),
              content: Text("Apakah Anda yakin ingin menghapus catatan ini?"),
              actions: [
                TextButton(
                  onPressed: () => Navigator.of(context).pop(false),
                  child: Text("Tidak"),
                ),
                TextButton(
                  onPressed: () => Navigator.of(context).pop(true),
                  child: Text("Ya"),
                ),
              ],
            );
          },
        );
      },
      onDismissed: (direction) {
        onDelete();
      },
      background: Container(
        alignment: Alignment.centerLeft,
        padding: EdgeInsets.symmetric(horizontal: 16),
        color: Colors.red,
        child: Icon(Icons.delete, color: Colors.white),
      ),
      child: Container(
        width: double.infinity,
        child: Card(
          margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  tanggal,
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold, color: const Color.fromARGB(255,0,101,31)),
                ),
                SizedBox(height: 4),
                Text(
                  waktu,
                  style: TextStyle(fontSize: 14, color: Color.fromARGB(255,0,101,31)),
                ),
                SizedBox(height: 8),
                Text(
                  catatan,
                  style: TextStyle(fontSize: 14, color: Colors.black),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
