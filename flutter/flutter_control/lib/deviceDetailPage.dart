import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

const String apiBase = "http://localhost:3000";

class DeviceDetailPage extends StatefulWidget {
  final String id;

  const DeviceDetailPage({super.key, required this.id});

  @override
  State<DeviceDetailPage> createState() => _DeviceDetailPageState();
}

class _DeviceDetailPageState extends State<DeviceDetailPage> {
  double brightness = 5.0; // 默认亮度（0–10）
  String mode = "manual"; // 当前模式：auto 或 manual

  Future<void> sendControl({
    required String action,
    bool? power,
    double? brightness,
    String? mode,
    bool? markMax,
    bool? markMin,
  }) async {
    final url = Uri.parse("$apiBase/${widget.id}/control");

    final body = {
      "action": action,
      "payload": {
        "power": power,
        "brightness": brightness,
        "mode": mode,
        "markMax": markMax,
        "markMin": markMin,
      }
    };

    // 删除 null 字段（后端不需要）
    (body["payload"] as Map<String, dynamic>?)?.removeWhere((key, value) => value == null);

    try {
      final res = await http.post(
        url,
        headers: {"Content-Type": "application/json"},
        body: jsonEncode(body),
      );

      print("Response: ${res.body}");
    } catch (e) {
      print("Error: $e");
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text("设备详情 - ${widget.id}")),
      body: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          children: [
            // ---------- 开关按钮 ----------
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  onPressed: () => sendControl(
                    action: "on",
                    power: true,
                  ),
                  child: const Text("开"),
                ),
                ElevatedButton(
                  onPressed: () => sendControl(
                    action: "off",
                    power: false,
                  ),
                  child: const Text("关"),
                ),
              ],
            ),

            const SizedBox(height: 20),

            // ---------- 自动 / 手动 模式 ----------
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  onPressed: () {
                    setState(() => mode = "auto");
                    sendControl(
                      action: "mode",
                      mode: "auto",
                    );
                  },
                  child: const Text("自动"),
                ),
                ElevatedButton(
                  onPressed: () {
                    setState(() => mode = "manual");
                    sendControl(
                      action: "mode",
                      mode: "manual",
                    );
                  },
                  child: const Text("手动"),
                ),
              ],
            ),

            const SizedBox(height: 30),

            // ---------- 亮度滑动条 ----------
            Text("亮度：${brightness.toStringAsFixed(1)}"),
            Slider(
              value: brightness,
              min: 0,
              max: 10,
              divisions: 100,
              label: brightness.toStringAsFixed(1),
              onChanged: (value) {
                setState(() => brightness = value);
              },
              onChangeEnd: (value) {
                sendControl(
                  action: "dim",
                  brightness: value,
                );
              },
            ),

            const SizedBox(height: 20),

            // ---------- 最亮 / 最暗 标记 ----------
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                OutlinedButton(
                  onPressed: () => sendControl(
                    action: "dim",
                    markMax: true,
                  ),
                  child: const Text("标记最亮"),
                ),
                OutlinedButton(
                  onPressed: () => sendControl(
                    action: "dim",
                    markMin: true,
                  ),
                  child: const Text("标记最暗"),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}
