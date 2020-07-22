package rbs.booking;

import java.util.ArrayList;
import java.util.List;
import java.time.LocalDate;

import rbs.booking.Room.RoomSize;

/**
 * Manages all reservations in the location hire system. This class keeps track
 * of all reservations, and is responsible for making, changing and cancelling
 * reservations
 * 
 * @author Stratton Sloane
 *
 */
public class ReservationManager {

	private LocationManager locationManager;
	private List<Reservation> reservations;

	/**
	 * Constructs a new reservation manager, initially with no reservations
	 * 
	 * @param locationManager The location manager associated with the system
	 */
	public ReservationManager(LocationManager locationManager) {
		reservations = new ArrayList<Reservation>();
		this.locationManager = locationManager;
	}

	/**
	 * Attempts to make a reservation with the given parameters, returning the
	 * resulting Reservation object if successful
	 * 
	 * @param id        The unique ID of the reservation
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @param small     The number of small rooms required
	 * @param medium    The number of medium rooms required
	 * @param large     The number of large rooms required
	 * @return A reservation object if the request could be fulfilled, otherwise
	 *         null
	 */
	public Reservation makeReservation(String id, LocalDate startDate, LocalDate endDate, int small, int medium,
			int large) {

		Reservation reservation = new Reservation(id, startDate, endDate);

		Location available = null;
		int sRemaining = small;
		int mRemaining = medium;
		int lRemaining = large;

		// Check if location can fulfill reservation request
		for (Location v : locationManager.getLocations()) {
			if (v.hasAvailableRooms(startDate, endDate, small, medium, large)) {
				available = v;
				break;
			}
		}

		if (available != null) {
			reservation.setLocation(available.getName());

			// Go through each room, and add to reservation as required
			for (Room r : available.getAvailableRooms(startDate, endDate)) {
				RoomSize size = r.getSize();
				String room = r.getName();

				if (sRemaining > 0 && size == RoomSize.Small) {
					sRemaining--;
					reservation.addRoom(room);
				}
				if (mRemaining > 0 && size == RoomSize.Medium) {
					mRemaining--;
					reservation.addRoom(room);
				}
				if (lRemaining > 0 && size == RoomSize.Large) {
					lRemaining--;
					reservation.addRoom(room);
				}
			}

			addReservation(reservation);
			return reservation;
		}

		return null;
	}

	/**
	 * Attempts to change an existing reservation, returning the new Reservation
	 * object if successful
	 * 
	 * @param id        The unique ID of the reservation
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @param small     The number of small rooms required
	 * @param medium    The number of medium rooms required
	 * @param large     The number of large rooms required
	 * @return A reservation object if the request could be fulfilled, otherwise
	 *         null
	 */
	public Reservation changeReservation(String id, LocalDate startDate, LocalDate endDate, int small, int medium,
			int large) {

		Reservation old = getReservation(id);
		if (old == null)
			return null;

		// Remove existing reservation from system
		removeReservation(old);
		// Attempt to create new reservation
		Reservation reservation = makeReservation(id, startDate, endDate, small, medium, large);

		if (reservation != null)
			return reservation;
		else {
			// Restore old reservation
			addReservation(old);
			return null;
		}
	}

	/**
	 * Attempts to cancel the reservation with the given ID
	 * 
	 * @param id The ID of the reservation
	 * @return True, or False if the reservation could not be found
	 */
	public boolean cancelReservation(String id) {
		Reservation r = getReservation(id);

		if (r != null) {
			removeReservation(r);
			return true;
		}

		return false;
	}

	/**
	 * Adds a reservation to the list of reservations, and additionally associates
	 * it with each room
	 * 
	 * @param reservation The Reservation object to add
	 */
	private void addReservation(Reservation reservation) {
		if (!reservations.contains(reservation))
			reservations.add(reservation);

		for (Room r : getReservationRooms(reservation)) {
			r.addReservation(reservation);
		}
	}

	/**
	 * Returns the reservation with the given ID
	 * 
	 * @param id The ID of the reservation
	 * @return The reservation, as a Reservation object
	 */
	private Reservation getReservation(String id) {
		for (Reservation r : reservations) {
			if (r.getID().equals(id))
				return r;
		}

		return null;
	}

	/**
	 * Removes the given reservation from the list of reservations
	 * 
	 * @param reservation A Reservation object
	 */
	private void removeReservation(Reservation reservation) {
		reservations.remove(reservation);

		for (Room r : getReservationRooms(reservation)) {
			r.removeReservation(reservation.getID());
		}
	}

	/**
	 * Given a reservation, returns all rooms associated with the reservation
	 * 
	 * @param reservation A Reservation object
	 * @return The list of Room objects associated with the reservation
	 */
	private List<Room> getReservationRooms(Reservation reservation) {
		List<Room> rooms = new ArrayList<Room>();
		Location v = locationManager.getLocation(reservation.getLocation());

		for (String room : reservation.getRooms())
			rooms.add(v.getRoom(room));

		return rooms;
	}
}
