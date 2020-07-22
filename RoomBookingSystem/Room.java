package rbs.booking;

import java.util.List;
import java.util.ArrayList;
import java.time.LocalDate;

/**
 * A room within a location, with a unique name and a size represented by a
 * RoomSize enum. Also contains each reservation associated with the room
 * 
 * Each room is responsible for determining whether it is available on a given
 * date range
 * 
 * @author Stratton Sloane
 *
 */
public class Room {

	public enum RoomSize {
		Small, Medium, Large
	};

	private String name;
	private RoomSize size;
	private List<Reservation> reservations;

	/**
	 * Constructs a new room with the given name and size
	 * 
	 * @param name A unique name for the room
	 * @param size A RoomSize enum (Small, Medium, or Large)
	 */
	public Room(String name, RoomSize size) {
		this.name = name;
		this.size = size;
		this.reservations = new ArrayList<Reservation>();
	}

	/**
	 * 
	 * @return The name of the room
	 */
	public String getName() {
		return name;
	}

	/**
	 * 
	 * @return The size of the room, as a RoomSize enum
	 */
	public RoomSize getSize() {
		return size;
	}

	/**
	 * 
	 * @return A list containing all reservations associated with the room
	 */
	public List<Reservation> getReservations() {
		return reservations;
	}

	/**
	 * 
	 * @return A list containing all reservations, ordered by start date
	 */
	public List<Reservation> getOrderedReservations() {
		List<Reservation> ordered = new ArrayList<Reservation>(reservations);
		ordered.sort((r1, r2) -> r1.getStartDate().compareTo(r2.getStartDate()));
		return ordered;
	}

	/**
	 * Adds a reservation to the reservation list
	 * 
	 * @param reservation A Reservation object
	 */
	public void addReservation(Reservation reservation) {
		reservations.add(reservation);
	}

	/**
	 * Removes a reservation from the reservation list
	 * 
	 * @param id The unique ID of the reservation
	 */
	public void removeReservation(String id) {
		reservations.removeIf(r -> (r.getID().equals(id)));
	}

	/**
	 * Determines whether the room is free in the given date range
	 * 
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @return True if the date range is free, false otherwise
	 */
	public boolean isAvailable(LocalDate startDate, LocalDate endDate) {
		for (Reservation r : reservations) {
			LocalDate start = r.getStartDate();
			LocalDate end = r.getEndDate();

			if (datesWithin(start, end, startDate, endDate))
				return false;
			if (datesWithin(startDate, endDate, start, end))
				return false;
		}

		return true;
	}

	/**
	 * Determines whether the dates of range A are within those of range B
	 * 
	 * @param startA The start date of range A
	 * @param endA   The end date of range A
	 * @param startB The start date of range B
	 * @param endB   The end date of range B
	 * @return True if range A is within range B, false otherwise
	 */
	private boolean datesWithin(LocalDate startA, LocalDate endA, LocalDate startB, LocalDate endB) {
		if (startA.compareTo(startB) >= 0 && startA.compareTo(endB) <= 0)
			return true;
		if (endA.compareTo(startB) >= 0 && endA.compareTo(endB) <= 0)
			return true;

		return false;
	}
}
