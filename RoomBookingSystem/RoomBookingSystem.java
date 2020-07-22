/**
 *
 */
package rbs.booking;

import java.time.LocalDate;
import java.util.Scanner;
import rbs.locations.Room.RoomSize;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Room Booking System
 *
 * @author Stratton Sloane
 *
 */
public class RoomBookingSystem {

	private LocationManager locationManager;
	private ReservationManager reservationManager;

	/**
	 * Constructs a room booking system. Initially, the system contains no locations,
	 * rooms, or bookings.
	 */
	public RoomBookingSystem() {
		locationManager = new LocationManager();
		reservationManager = new ReservationManager(locationManager);
	}

	/**
	 * Processes a command in JSON format
	 * 
	 * @param json The JSONObject to process
	 */
	private void processCommand(JSONObject json) {
		JSONObject result = null;

		switch (json.getString("command")) {
			case "room": {
				String location = json.getString("location");
				String room = json.getString("room");
				String size = json.getString("size");

				addRoom(location, room, size);
				break;
			}
			case "request": {
				String id = json.getString("id");
				LocalDate start = LocalDate.parse(json.getString("start"));
				LocalDate end = LocalDate.parse(json.getString("end"));
				int small = json.getInt("small");
				int medium = json.getInt("medium");
				int large = json.getInt("large");

				result = request(id, start, end, small, medium, large);
				break;
			}
			case "change": {
				String id = json.getString("id");
				LocalDate start = LocalDate.parse(json.getString("start"));
				LocalDate end = LocalDate.parse(json.getString("end"));
				int small = json.getInt("small");
				int medium = json.getInt("medium");
				int large = json.getInt("large");

				result = change(id, start, end, small, medium, large);
				break;
			}
			case "cancel": {
				result = cancel(json.getString("id"));
				break;
			}
			case "list": {
				JSONArray rooms = listRooms(json.getString("location"));
				System.out.println(rooms.toString(2));
				break;
			}
		}

		if (result != null)
			System.out.println(result.toString(0));
	}

	/**
	 * Adds a room to the system at a location, with a given name and size
	 * 
	 * @param location The location the room belongs to
	 * @param room  The name of the room
	 * @param size  The size of the room (small, medium, large)
	 */
	private void addRoom(String location, String room, String size) {
		Location v;
		RoomSize s = null;

		if (size.equals("small"))
			s = RoomSize.Small;
		else if (size.equals("medium"))
			s = RoomSize.Medium;
		else if (size.equals("large"))
			s = RoomSize.Large;
		else
			error("Invalid room size");

		v = locationManager.getLocation(location);
		if (v == null)
			v = locationManager.addLocation(location);

		v.addRoom(new Room(room, s));
	}

	/**
	 * Returns a JSON array that lists the rooms of a location, and their reservations
	 * 
	 * @param location The name of the location to query
	 * @return A JSON array containing room details
	 */
	private JSONArray listRooms(String location) {
		JSONArray rooms = new JSONArray();
		Location v = locationManager.getLocation(location);

		// Iterate through each room in location
		for (Room r : v.getRooms()) {
			JSONObject room = new JSONObject();
			room.put("room", r.getName());

			JSONArray reservations = new JSONArray();

			// Add reservations in date order
			for (Reservation res : r.getOrderedReservations()) {
				JSONObject reservation = new JSONObject();

				reservation.put("id", res.getID());
				reservation.put("start", res.getStartDate());
				reservation.put("end", res.getEndDate());

				reservations.put(reservation);
			}

			room.put("reservations", reservations);
			rooms.put(room);
		}

		return rooms;
	}

	/**
	 * Attempts to make a reservation, given required attributes
	 * 
	 * @param id        The unique ID of the reservation
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @param small     The number of small rooms required
	 * @param medium    The number of medium rooms required
	 * @param large     The number of large rooms required
	 * @return A JSONObject indicating success or rejection, and the location and rooms
	 *         booked if applicable
	 */
	private JSONObject request(String id, LocalDate start, LocalDate end, int small, int medium, int large) {
		Reservation r = reservationManager.makeReservation(id, start, end, small, medium, large);
		return outputReservation(r);
	}

	/**
	 * Attempts to change a reservation, given required attributes
	 * 
	 * @param id        The unique ID of the reservation
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @param small     The number of small rooms required
	 * @param medium    The number of medium rooms required
	 * @param large     The number of large rooms required
	 * @return A JSONObject indicating success or rejection, and the location and rooms
	 *         booked if applicable
	 */
	private JSONObject change(String id, LocalDate start, LocalDate end, int small, int medium, int large) {
		Reservation r = reservationManager.changeReservation(id, start, end, small, medium, large);
		return outputReservation(r);
	}

	/**
	 * Given a Reservation object, outputs the status of a reservation request as a
	 * JSONObject
	 * 
	 * @param reservation The Reservation object
	 * @return A JSONObject indicating success or rejection, and the location and rooms
	 *         booked if applicable
	 */
	private JSONObject outputReservation(Reservation reservation) {
		JSONObject result = new JSONObject();

		if (reservation == null) {
			result.put("status", "rejected");
			return result;
		} else {
			result.put("status", "success");
			result.put("location", reservation.getLocation());
			JSONArray rooms = new JSONArray();

			for (String room : reservation.getRooms()) {
				rooms.put(room);
			}

			result.put("rooms", rooms);

			return result;
		}
	}

	/**
	 * Attempts to cancel the given reservation
	 * 
	 * @param id The unique ID of the reservation
	 * @return A JSONObject indicating success or rejection
	 */
	private JSONObject cancel(String id) {
		JSONObject result = new JSONObject();

		boolean success = reservationManager.cancelReservation(id);

		if (success)
			result.put("status", "success");
		else
			result.put("status", "rejected");

		return result;
	}

	/**
	 * Prints an error message, then exits the application
	 * 
	 * @param message The message to print
	 */
	private void error(String message) {
		System.out.println(message);
		System.exit(1);
	}

	public static void main(String[] args) {
		RoomBookingSystem system = new RoomBookingSystem();

		Scanner sc = new Scanner(System.in);

		while (sc.hasNextLine()) {
			String line = sc.nextLine();
			if (!line.trim().equals("")) {
				JSONObject command = new JSONObject(line);
				system.processCommand(command);
			}
		}
		sc.close();
	}

}
