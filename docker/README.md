# rawdrawandroid Docker image

Docker image containing all tools for building rawdrawandroid projects. Host system needs to have only `adb` installed, everything else is inside Docker image. Host system `adb` is used as a server and `adb` inside running container is connected to this server in client mode using `adb -H host.docker.internal` parameter.

## Usage in Windows

* You need to have `adb` installed on your host Windows. `adb` **versions on host system and inside Docker container MUST be the same**. Usually rebuilding Docker image and updating platform-tools on host system will ensure equal `adb` versions.
* Edit `rda.ps1` helper script and update `$CmdAdb` so that it points to `adb` executable on your system.
* Connect Android device to host system and check that host `adb` can detect it:

```powershell
> adb devices -l
List of devices attached
bcc4c4e1               device product:lineage_TBX704 model:Lenovo_TB_X704F device:X704F transport_id:3
```

* Build Docker image:

```powershell
cd docker
.\rda.ps1 -Build
cd ..
```

* Check that `adb` inside container can connect to host `adb` and your Android device:

```powershell
> .\docker\rda.ps1 -Run adb devices -l
List of devices attached
bcc4c4e1               device product:lineage_TBX704 model:Lenovo_TB_X704F device:X704F transport_id:3
```

### Running commands

Run `make` commands in powershell console by prefixing them with `.\docker\rda.ps1 -Run`. Adjust path to `rda.ps1` as necessary or copy `rda.ps1` to your project folder.

You can either run container separately for every command:

```powershell
> .\docker\rda.ps1 -Run make run
```

Or you can preload container and leave it running in background. This way you will avoid delays in creating and tearing down of the container.

```powershell
> .\docker\rda.ps1 -Start
```

Execute as many commands as you need:

```powershell
> .\docker\rda.ps1 -Exec make run
```

And finally stop container:

```powershell
> .\docker\rda.ps1 -Stop
```

Note: Commands executed using both `-Run` and `-Exec` are run in the current working directory.
