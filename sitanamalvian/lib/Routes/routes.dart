import 'package:flutter/material.dart';
import 'package:sitanamalvian/Pages/dashboard.dart';
import 'package:sitanamalvian/Pages/splashscreen.dart';
import 'package:sitanamalvian/Pages/dashboard.dart';
import 'package:sitanamalvian/Pages/settings.dart';
import 'package:sitanamalvian/Pages/addcatatan.dart';
import 'package:sitanamalvian/Pages/catatan.dart';


class Routes {
  static const String splash = '/';
  static const String dashboard = '/dashboard';
  static const String settings = '/settings';
  static const String addCatatan = '/addcatatan';

  static Map<String, WidgetBuilder> routes = {
    splash: (context) => SplashScreen(),
    dashboard: (context) => DashboardScreen(),
    settings: (context) => SettingsPage(),
     addCatatan: (context) => AddCatatanScreen(
      selectedPlot: ModalRoute.of(context)!.settings.arguments as String,
    ),
  };
}