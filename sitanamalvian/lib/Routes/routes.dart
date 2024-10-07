import 'package:flutter/material.dart';
import 'package:sitanamalvian/Pages/dashboard.dart';
import 'package:sitanamalvian/Pages/splashscreen.dart';
import 'package:sitanamalvian/Pages/dashboard.dart';
import 'package:sitanamalvian/Pages/settings.dart';


class Routes {
  static const String splash = '/';
  static const String dashboard = '/dashboard';
  static const String settings = '/settings';

  static Map<String, WidgetBuilder> routes = {
    splash: (context) => SplashScreen(),
    dashboard: (context) => DashboardScreen(),
    settings: (context) => SettingsPage(),
  };
}