<div align="center">

# DDNet-Vebury

[![GitHub Repository](https://img.shields.io/badge/GitHub-Repository-181717?style=for-the-badge&logo=github)](https://github.com/fkinaas-md/DDNet-Vebury)

</div>

**DDNet-Vebury** is a custom client fork of **DDraceNetwork** featuring a built-in interactive **Admin Panel**. The panel is integrated directly into the client settings and can be accessed via **Settings ➔ Assets ➔ Vebury**.

---


![Admin Panel](screenshots/admin_panel.png)

![Log](screenshots/log.png)

### 🛠 Key Features

#### 🛡️ 1. "Admin Panel" Tab
Streamline server moderation with a powerful and fully interactive interface:

*   **Interactive Player List (Left Side)**
    *   Features cute **Tees whose eyes dynamically follow your mouse cursor** in real time!
    *   Displays detailed client versions retrieved automatically via the `rcon status` command.
*   **Bulk Player Moderation**
    *   Select multiple players at once for batch operations.
    *   Displays real-time **selection counts** so you always know how many players are selected.
*   **Deselect All Button**
    *   A single-click button to quickly clear your selection.
*   **Preset Reasons**
    *   Quick-apply predefined infraction reasons: `flood`, `bot`, or `cheat`.
*   **Preset Durations**
    *   Easily choose predefined ban/mute durations:
        *   `3600m` (3600 minutes)
        *   `300s`, `500s`, `700s` (seconds)
*   **Instant Action Buttons**
    *   Apply administrative power instantly with the **Ban**, **Kick**, and **Mute** buttons.

#### 📋 2. "Log" Tab
Keep track of administrative actions with a built-in logging system:
*   Displays comprehensive logs including:
    *   **Client Version** of the moderated player
    *   **IP Address** (fetched via `show_ips 1` / `status`)
    *   **Target Name**
    *   **Reason**
    *   **Duration**
    *   **Action Type** (Ban, Kick, Mute, etc.)

---

### 💻 Compilation

To compile **DDNet-Vebury** yourself, use the following instructions depending on your operating system:

#### **Linux / macOS**
Ensure you have installed the required dependencies, then execute:
```sh
cmake -Bbuild -GNinja
cmake --build build
```
