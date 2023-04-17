# Project 2 - IOTA: Server for Remote Calculator

## Usage

`ipkcpd` is a C# command-line application that starts a server listening on a specified IPv4 address and port. It supports two modes of operation: TCP and UDP:

```ipkcpd -h <host> -p <port> -m <mode: udp|tcp>```

Where host is IPv4 address, port is port and mode must be specified either as `udp` or as `tcp`.

## Build

Project can be built using `make` which executes `dotnet` to build the desired project.

## Make

Makefile contains options:
- `make` - default make, builds and publishes ipkcpd project,
- `make build` - builds ipkcpd project,
- `make test` - builds and runs ipkcpd.Tests.

## Implementation

The project contains two subprojects: `ipkcpd` and `ipkcpd.Tests`.

### ipkcpd

ipkcpd is a server that uses `IPK Calculator Protocol` and serves as a calculator.
The server can be run either as UDP server or as TCP server.

#### Main body

Class `Program` contains Main method which contains argument parsing using `System.CommandLine` NuGet package from Microsoft (only in prerelease) and based on specified mode runs either `TcpServer` or `UdpServer` class.
The main body also contains SIGINT handler, which, based on specified mode, tries to dispose of the currently running server and all its resources (which in case of TCP includes sending last message "BYE" to all connected clients) before shutting down.

#### UdpServer class

`UdpServer` class contains implementation of UDP server. It uses `UdpClient` from `System.Net.Sockets` namespace. First, it validates that first and second byte are as specified in `IPK Calculator Protocol` and then proceeds with calling class `ExpressionParser` to solve the given expression. In case of an exception it sends back a message with information of the error that was found in the specified message or expression.

#### TcpServer class

`TcpServer` class contains partial implementation of TCP server. It uses `TcpListener` from `System.Net.Sockets` namespace. It awaits incomming clients and after a client joins it is send to an asynchronously running task which calls class `TcpClientHandler`. This cycle continues till the server receives SIGINT. This enables the TCP server to have joined multiple clients at the same time. If SIGINT is received, all of those handlers are disposed (the dispose method of each handler contains implementation of sending the last "BYE" to the specified client before closing the connection).

#### TcpClientHandler

`TcpClientHandler` class contains implementation for servicing of the specific client. First it awaits "HELLO" from the client and after that proceeds with parsing sent queries. In case of an invalid message the handler sends "BYE" and closes the connection with the specific client.

#### ExpressionParser

`ExpressionParser` class contains implementation for parsing and solving given expression based on message format specified in `IPK Calculator Protocol`. First it validates the expression and then continues with solving it. In case of any error, exception `InvalidExpressionException` is thrown with corresponding discription.

### ipkcpd.Tests

Testing was done using XUnit.net Framework.
Tests are split into tests for `ExpressionParser` (valid and invalid inputs), `TcpServer` and `UdpServer`. 

#### Testing

The tests:

| Test Name | Result | Time Elapsed |
| --- | --- | --- |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: " (+ 1 2)") | Passed | 3 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(* 1 2 3 4 5)", expected: 120) | Passed | 5 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "+ 1 2)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "      ") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(+ (- 2 1) (* 1 1))", expected: 2) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(- (* 2 3 4) (/ 16 2) (+ 5 4))", expected: 7) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(/ 5 (- 3 3))") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(* 3 4)", expected: 12) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(/ 1000 5 2 2 5)", expected: 10) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(+ 1 2) ") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(/ (+ (/ (+ (/ 121 (* (/ 2048 64 2) 5 3 2 18)) (* ", expected: 3) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(+ (/ 10 2) (* (- 3 4) (+ 1 2)))", expected: 2) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(+ 1 )") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(- 12 4 2 1)", expected: 5) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(1 2)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(- 100 50 20 10)", expected: 20) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(+ 2 2)", expected: 4) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(+ 1)") | Passed | < 1 ms |
| MessageWithTwoLines_CorrectResults | Passed | 8 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(* (+ 3 4 5) (- 8 6) (/ 20 4))", expected: 120) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(+ 5 (* 3 2))", expected: 11) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(*    (* 4 5  (* 4 5) )     ") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(* 2 3 4)", expected: 24) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(/ 0 0)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(- 7 3 1)", expected: 3) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(/ 100 2 2 5)", expected: 5) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(* - 4 5)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(/ (- 10 6) (+ 1 2 3) (* 3 4))", expected: 0) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(* 2 3 4 5)", expected: 120) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(/ 5 0)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(- 10 (/ 20 2))", expected: 0) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(+ 1 (* 2 3) (/ 4 2))", expected: 9) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(+ 1 2") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(/ 200 (+ 100 50) (* 2 2))", expected: 0) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(/ 10 2)", expected: 5) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(/ 0 (- 3 3))") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(+ (* 2 3) (/ 8 2) (- 10 6))", expected: 14) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput._ThrowInvalidExpressionException(expr: "(   *    4   5   )     ") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ReturnExpectedResult(input: "(* (- 5 2) (+ 1 2 3) (/ 18 3))", expected: 108) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(* 2 (+ 3 4) (- 5 1))", expected: 56) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(* -4 5)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(+ 1 (/ 5 5))", expected: 2) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(*(/ 4 4))HELLO") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(/ 16 2 2)", expected: 4) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "(* --4 5)") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(+ 2 (* 3 4) (- 10 6))", expected: 18) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(* (- 4 2) (+ 3 4) (/ 10 2))", expected: 70) | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsInvalidInput.InvalidInput_ThrowInvalidExpressionException(expr: "") | Passed | < 1 ms |
| ipkcpd.Tests.ExpressionSolverTests.ExpressionSolverTestsValidInput.ValidInputs_ReturnExpectedResult(input: "(- 8 5)", expected: 3) | Passed | < 1 ms |
| ipkcpd.Tests.Server.TcpServer.TcpServerTests.MultipleClients_NotThrowOrDisconnect | Passed | 928 ms |
| ipkcpd.Tests.Server.UdpServer.UdpServerTests.SendInvalidMessage_Receive1OnSecondIndex | Passed | 949 ms |
| ipkcpd.Tests.Server.UdpServer.UdpServerTests.SendCorrectMessage_Receive0OnSecondIndex | Passed | 104 ms |
| ipkcpd.Tests.Server.TcpServer.TcpServerTests.SendInvalidMessage_GetBye | Passed | 999 ms |
| ipkcpd.Tests.Server.TcpServer.TcpServerTests.SendFirstHello_GetHello | Passed | 1 s |
| ipkcpd.Tests.Server.TcpServer.TcpServerTests.DisconnectClientWithoutBye_NotThrow | Passed | 1 ms |
| ipkcpd.Tests.Server.TcpServer.TcpServerTests.TwoCorrectExpressionsInARow_CorrectResults | Passed | 1 ms |
| Test Run Successful. |  |  |
| Total tests: 56 |  |  |
| Passed: 56 |  |  |
| Total time: 4.6924 Seconds |  |  |


## References

[System.Net.Sockets] https://learn.microsoft.com/cs-cz/dotnet/api/system.net.sockets.socket?view=net-7.0
[XUnit] https://xunit.net/ Used testing framework
[IPK Calculator Protocol] https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md

