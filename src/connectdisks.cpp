#include "connectdisks/connectdisks.hpp"

#include "logging.hpp"

#include <exception>
#include <iostream>
#include <sstream>

using namespace connectdisks;

ConnectDisks::ConnectDisks(
	Board::player_size_t numPlayers,
	Board::player_size_t firstPlayer,
	Board::board_size_t numColumns,
	Board::board_size_t numRows) :
	numTurns{0},
	numPlayers{numPlayers >= minNumPlayers ? numPlayers : minNumPlayers},
	currentPlayer{firstPlayer > defaultFirstPlayer && firstPlayer <= numPlayers ? firstPlayer : defaultFirstPlayer},
	lastMove{0, 0},
	winner{noWinner},
	board{numColumns, numRows}
{
}

ConnectDisks::ConnectDisks(ConnectDisks && connect) noexcept :
	numTurns{connect.numTurns},
	numPlayers{connect.numPlayers},
	currentPlayer{connect.currentPlayer},
	lastMove{std::move(connect.lastMove)},
	winner{connect.winner},
	board{std::move(connect.board)}
{
	connect.numTurns = 0;
	connect.numPlayers = 0;
	connect.currentPlayer = 0;
	connect.lastMove.first = 0;
	connect.lastMove.second = 0;
	connect.winner = noWinner;
}

ConnectDisks::~ConnectDisks()
{
}

ConnectDisks::TurnResult ConnectDisks::takeTurn(Board::player_size_t player, Board::board_size_t column)
{
	if (winner != noWinner)
	{
		return TurnResult::gameFinished;
	}

	if (currentPlayer != player)
	{
		return TurnResult::wrongPlayer;
	}

	try
	{
		bool dropSuccessful = board.dropPieceInColumn(column, player);
		if (!dropSuccessful)
		{
			return TurnResult::columnFull;
		}
		else
		{
			// turn was valid
			currentPlayer = getNextPlayer();
			lastMove = std::make_pair<>(column, board.getColumnHeight(column) - 1);
			++numTurns;
			winner = checkForWinner();
		}
	}
	catch (std::out_of_range& error)
	{
		// invalid column number 
		printDebug("ConnectDisks::takeTurn: error taking turn:", error.what(), "\n");
		return TurnResult::badColumn;
	}
	catch (std::exception& error)
	{
		printDebug("ConnectDisks::takeTurn: fatal error taking turn",error.what(),"\n");
		throw;
	}

	return TurnResult::success;
}

ConnectDisks & ConnectDisks::operator=(ConnectDisks && connect) noexcept
{
	board = std::move(connect.board);
	numTurns = connect.numTurns;
	numPlayers = connect.numPlayers;
	lastMove = std::move(connect.lastMove);
	winner = connect.winner;
	currentPlayer = connect.currentPlayer;
	connect.numTurns = 0;
	connect.numPlayers = 0;
	connect.lastMove.first = 0;
	connect.lastMove.second = 0;
	connect.winner = noWinner;
	connect.currentPlayer = 0;
	return *this;
}

Board::player_size_t connectdisks::ConnectDisks::checkForWinner() const
{
	const Board::board_size_t lastColumn{lastMove.first};
	const Board::board_size_t lastRow{lastMove.second};

	// assume the chain starts with the last move, and only check if
	// the last player won
	size_t count{1};
	const Board::player_size_t lastPlayer{board.getDiskOwnerAt(lastColumn, lastRow)};
	const Board::column_view_t column{board.getColumn(lastColumn)};

	// check up the column (0 would be bottom, numRows - 1 would be top), starting above
	// the last move
	for (Board::board_size_t row = lastRow + 1; row < board.getNumRows(); ++row)
	{
		Board::player_size_t current = column[row];
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	// check down the column, starting below the last move
	for (Board::board_size_t row = lastRow - 1; row != static_cast<Board::board_size_t>(-1); --row)
	{
		Board::player_size_t current = column[row];
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	// check across the row
	count = 1;
	const Board::column_value_t row{board.getRow(lastRow)};

	// scan row left to right, starting one to the right of last move
	for (Board::board_size_t col = lastColumn + 1; col < board.getNumColumns(); ++col)
	{
		Board::player_size_t current = row[col];
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	// check right to left, starting one to the left of last move
	for (Board::board_size_t col = lastColumn - 1; col != static_cast<Board::board_size_t>(-1); --col)
	{
		Board::player_size_t current = row[col];
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	count = 1;

	// check up-right diagonal
	for (Board::board_size_t row = lastRow + 1, col = lastColumn + 1;
		row < board.getNumRows() && col < board.getNumColumns();
		++row, ++col)
	{
		Board::player_size_t current = board.getDiskOwnerAt(col, row);
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	// check down-left diagonal
	for (Board::board_size_t row = lastRow - 1, col = lastColumn - 1;
		row != static_cast<Board::board_size_t>(-1) && col != static_cast<Board::board_size_t>(-1);
		--row, --col)
	{
		Board::player_size_t current = board.getDiskOwnerAt(col, row);
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	count = 1;

	// check up-left diagonal
	for (Board::board_size_t row = lastRow + 1, col = lastColumn - 1;
		row < board.getNumRows() && col != static_cast<Board::board_size_t>(-1);
		++row, --col)
	{
		Board::player_size_t current = board.getDiskOwnerAt(col, row);
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	// check down-right diagonal
	for (Board::board_size_t row = lastRow - 1, col = lastColumn + 1;
		row != static_cast<Board::board_size_t>(-1) && col < board.getNumColumns();
		--row, ++col)
	{
		Board::player_size_t current = board.getDiskOwnerAt(col, row);
		if (current != lastPlayer)
		{
			break;
		}
		else
		{
			++count;
			if (count == 4)
			{
				return lastPlayer;
			}
		}
	}

	return noWinner;
}

Board::player_size_t connectdisks::ConnectDisks::getNextPlayer() const noexcept
{
	Board::player_size_t nextPlayer = currentPlayer + 1; // may overflow to 0
	if (nextPlayer > numPlayers || nextPlayer == 0)
	{
		nextPlayer = defaultFirstPlayer; // wrap around
	}
	return nextPlayer;
}

connectdisks::ConnectDisks::operator std::string() const
{
	std::ostringstream strStream;
	strStream << board << "\n" <<
		"Current player: " << static_cast<int>(currentPlayer) <<
		" Next player: " << static_cast<int>(getNextPlayer()) <<
		"\nNum turns: " << static_cast<int>(numTurns) << "\n";
	if (hasWinner())
	{
		strStream << "Winner: " << static_cast<int>(winner) << "\n";
	}
	return strStream.str();
}