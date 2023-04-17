using System.CommandLine;
using System.Threading.Tasks;

namespace ipkcpd;

public static class Program
{
    private enum Mode
    {
        Tcp,
        Udp
    }

    private static Mode _mode;
    private static UdpServer? _udpServer;
    private static TcpServer? _tcpServer;

    private static int Main(string[] args)
    {
        var hostOption = new Option<string>(
            "-h",
            description: "The IPv4 address the server will listen on.")
        {
            IsRequired = true,
            Arity = ArgumentArity.ExactlyOne
        };
        var portOption = new Option<int>(
            "-p",
            description: "The port it will listen on.")
        {
            IsRequired = true,
            Arity = ArgumentArity.ExactlyOne
        };
        var modeOption = new Option<Mode>(
            "-m",
            description: "The mode either tcp or udp.")
        {
            IsRequired = true,
            Arity = ArgumentArity.ExactlyOne
        }.FromAmong("tcp", "udp");


        var rootCommand = new RootCommand
        {
            Description = "ipkcpd -h <host> -p <port> -m <mode>"
        };
        rootCommand.AddOption(hostOption);
        rootCommand.AddOption(portOption);
        rootCommand.AddOption(modeOption);

        rootCommand.SetHandler((host, port, mode) =>
        {
            _mode = mode;
            // Set up the console handler for SIGINT signal
            Console.CancelKeyPress += Console_CancelKeyPress;

            switch (mode)
            {
                case Mode.Tcp:
                    Console.WriteLine($"Starting TCP server with ip {host} and port {port}");
                    _tcpServer = new TcpServer();
                    _tcpServer.Run(host, port);
                    break;

                case Mode.Udp:
                    Console.WriteLine($"Starting UDP server with ip {host} and port {port}");
                    _udpServer = new UdpServer();
                    _udpServer.Run(host, port);
                    break;
            }
        }, hostOption, portOption, modeOption);

        return rootCommand.InvokeAsync(args).Result;
    }

    private static void Console_CancelKeyPress(object? sender, ConsoleCancelEventArgs e)
    {
        Console.WriteLine("Caught SIGINT.");

        switch (_mode)
        {
            case Mode.Tcp:
                Console.WriteLine("Disposing Tcp server and its connections.");
                _tcpServer?.Dispose();
                break;

            case Mode.Udp:
                Console.WriteLine("Disposing Udp server and its connections.");
                _udpServer?.Dispose();
                break;
        }

        Environment.Exit(0);
    }
}
